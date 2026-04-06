#include "machine_lowerer.hpp"
#include "expr_lowerer.hpp"
#include "stmt_lowerer.hpp"

namespace flux {

MachineLowerer::MachineLowerer(LoweringContext &ctx, ExprLowerer &expr_lowerer, StmtLowerer &stmt_lowerer)
    : _ctx(ctx), _expr_lowerer(expr_lowerer), _stmt_lowerer(stmt_lowerer) {}

LoweredMachine MachineLowerer::LowerMachine(const MachineDecl &machine) {
  LoweredMachine lowered_machine;
  lowered_machine.source = &machine;
  lowered_machine.name = machine.name;

  _ctx.EnterMachine(machine);
  for (const auto &[name, info] : _ctx.Semantic().MachineInfos()) {
    if (info.decl == nullptr) {
      continue;
    }
    _ctx.RegisterMachineGlobal(name);
    _ctx.BindSymbol(LoweredSymbolRef{
        .name = name,
        .type = LoweredType{.kind = LoweredTypeKind::MachineRef},
        .storage = LoweredStorageKind::MachineSymbol,
        .index = 0,
        .semantic_symbol =
            ResolvedSymbol{
                .kind = SymbolKind::MachineName,
                .name = name,
                .type = ValueType::MachineRef,
                .machine_decl = info.decl,
                .var_decl = nullptr,
                .param = nullptr,
            },
    });
  }
  LowerStates(machine, lowered_machine);
  LowerEvents(machine, lowered_machine);
  LowerEnv(machine, lowered_machine);
  lowered_machine.initial_state_tag = _ctx.LookupStateTag(machine.initial_state);
  LowerTransitions(machine, lowered_machine);
  _ctx.LeaveMachine();

  return lowered_machine;
}

void MachineLowerer::LowerStates(const MachineDecl &machine, LoweredMachine &lowered_machine) {
  for (size_t i = 0; i < machine.states.size(); ++i) {
    lowered_machine.states.push_back(LoweredState{
        .name = machine.states[i],
        .tag = static_cast<int32_t>(i),
    });
    _ctx.RegisterStateTag(machine.states[i], static_cast<int32_t>(i));
  }
}

void MachineLowerer::LowerEvents(const MachineDecl &machine, LoweredMachine &lowered_machine) {
  for (size_t i = 0; i < machine.events.size(); ++i) {
    LoweredEvent lowered_event;
    lowered_event.name = machine.events[i].name;
    lowered_event.tag = static_cast<int32_t>(i);
    for (const auto &param : machine.events[i].params) {
      lowered_event.params.push_back(LoweredField{
          .name = param.name,
          .type = LoweredTypeFromValueType(param.type),
      });
    }
    lowered_machine.events.push_back(std::move(lowered_event));
    _ctx.RegisterEventTag(machine.events[i].name, static_cast<int32_t>(i));
  }
}

void MachineLowerer::LowerEnv(const MachineDecl &machine, LoweredMachine &lowered_machine) {
  for (size_t i = 0; i < machine.vars.size(); ++i) {
    const auto &decl = machine.vars[i];
    lowered_machine.env.fields.push_back(LoweredField{
        .name = decl.name,
        .type = LoweredTypeFromValueType(decl.type),
    });

    LoweredSymbolRef symbol;
    symbol.name = decl.name;
    symbol.type = LoweredTypeFromValueType(decl.type);
    symbol.storage = LoweredStorageKind::EnvField;
    symbol.index = i;
    symbol.semantic_symbol = ResolvedSymbol{
        .kind = SymbolKind::MachineVar,
        .name = decl.name,
        .type = decl.type,
        .machine_decl = nullptr,
        .var_decl = &decl,
        .param = nullptr,
    };
    _ctx.BindSymbol(std::move(symbol));
  }

  for (const auto &decl : machine.vars) {
    if (decl.initializer) {
      lowered_machine.env.initializers.push_back(_expr_lowerer.LowerExpr(*decl.initializer));
    } else {
      lowered_machine.env.initializers.push_back(nullptr);
    }
  }
}

void MachineLowerer::LowerTransitions(const MachineDecl &machine, LoweredMachine &lowered_machine) {
  for (size_t i = 0; i < machine.transitions.size(); ++i) {
    lowered_machine.transitions.push_back(LowerTransition(machine.transitions[i], lowered_machine, i));
  }
}

LoweredTransition MachineLowerer::LowerTransition(const TransitionDecl &transition, LoweredMachine &lowered_machine,
                                                  size_t index) {
  LoweredTransition lowered_transition;
  lowered_transition.source = &transition;
  lowered_transition.src_state_tag = _ctx.LookupStateTag(transition.src_state);
  lowered_transition.dst_state_tag = _ctx.LookupStateTag(transition.dst_state);

  _ctx.PushScope();

  if (const auto *trigger = std::get_if<OnEventTrigger>(&transition.trigger)) {
    lowered_transition.trigger_kind = LoweredTriggerKind::OnEvent;
    if (const auto *event = _ctx.Semantic().FindOnEventResolution(*trigger)) {
      lowered_transition.trigger_id = _ctx.LookupEventTag(event->name);
      for (size_t i = 0; i < trigger->bindings.size() && i < event->params.size(); ++i) {
        LoweredSymbolRef symbol;
        symbol.name = trigger->bindings[i];
        symbol.type = LoweredTypeFromValueType(event->params[i].type);
        symbol.storage = LoweredStorageKind::TriggerValue;
        symbol.index = i;
        symbol.semantic_symbol = ResolvedSymbol{
            .kind = SymbolKind::TriggerBinding,
            .name = trigger->bindings[i],
            .type = event->params[i].type,
            .machine_decl = nullptr,
            .var_decl = nullptr,
            .param = &event->params[i],
        };
        lowered_transition.bindings.push_back(symbol);
        _ctx.BindSymbol(symbol);
      }
    } else {
      lowered_transition.trigger_id = _ctx.LookupEventTag(trigger->event_name);
      _ctx.Error("missing semantic event resolution for on-trigger '" + trigger->event_name + "'");
    }
  } else if (const auto *trigger = std::get_if<AfterTrigger>(&transition.trigger)) {
    lowered_transition.trigger_kind = LoweredTriggerKind::After;
    const auto timeout_event_tag = static_cast<int32_t>(lowered_machine.events.size());
    std::string timeout_event_name;
    if (const auto *machine = _ctx.CurrentMachine()) {
      timeout_event_name = "__timeout_" + machine->name + "_" + transition.src_state + "_" + std::to_string(index);
    } else {
      timeout_event_name = "__timeout_" + std::to_string(index);
    }
    lowered_transition.timer = LowerAfterTimer(transition, *trigger, timeout_event_tag, timeout_event_name);
    if (lowered_transition.timer.has_value()) {
      lowered_transition.trigger_id = lowered_transition.timer->timeout_event_tag;
      lowered_machine.events.push_back(LoweredEvent{
          .name = lowered_transition.timer->timeout_event_name,
          .tag = lowered_transition.timer->timeout_event_tag,
          .is_timeout = true,
      });
      _ctx.RegisterEventTag(lowered_transition.timer->timeout_event_name, lowered_transition.timer->timeout_event_tag);
    }
  }

  if (transition.guard) {
    lowered_transition.guard = _expr_lowerer.LowerExpr(*transition.guard);
  }
  if (transition.actions) {
    lowered_transition.action = _stmt_lowerer.LowerBlock(*transition.actions);
  }

  _ctx.PopScope();
  return lowered_transition;
}

std::optional<LoweredTimerSpec> MachineLowerer::LowerAfterTimer(const TransitionDecl & /*transition*/,
                                                                const AfterTrigger &trigger, int32_t timeout_event_tag,
                                                                std::string timeout_event_name) {
  LoweredTimerSpec spec;
  spec.delay = trigger.delay.duration;
  spec.unit = trigger.delay.unit;
  spec.retry = trigger.retry;
  spec.timeout_event_tag = timeout_event_tag;
  spec.timeout_event_name = std::move(timeout_event_name);
  return spec;
}

} // namespace flux
