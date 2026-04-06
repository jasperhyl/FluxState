#include "machine_analyzer.hpp"
#include "expr_analyzer.hpp"
#include "stmt_analyzer.hpp"
#include <algorithm>
#include <variant>

namespace flux {

MachineAnalyzer::MachineAnalyzer(Context &ctx, ExprAnalyzer &expr_analyzer, StmtAnalyzer &stmt_analyzer,
                                 SemanticModel &semantic_model)
    : _ctx(ctx), _expr_analyzer(expr_analyzer), _stmt_analyzer(stmt_analyzer), _semantic_model(semantic_model) {}

void MachineAnalyzer::AnalyzeMachine(const MachineDecl &machine) {
  _ctx.EnterMachine(machine);

  AnalyzeInitialState(machine);
  DeclareMachineVars(machine);
  AnalyzeMachineVarInitializers(machine);

  for (const auto &transition : machine.transitions) {
    AnalyzeTransition(transition);
  }

  _ctx.LeaveMachine();
}

void MachineAnalyzer::AnalyzeInitialState(const MachineDecl &machine) {
  if (machine.initial_state.empty()) {
    _ctx.Error("machine '" + machine.name + "' is missing an initial state");
    return;
  }

  if (!_ctx.HasState(machine.name, machine.initial_state)) {
    _ctx.Error("initial state '" + machine.initial_state + "' is not declared in machine '" + machine.name + "'");
  }
}

void MachineAnalyzer::DeclareMachineVars(const MachineDecl &machine) {
  for (const auto &decl : machine.vars) {
    _ctx.DeclareMachineVarSymbol(decl);
  }
}

void MachineAnalyzer::AnalyzeMachineVarInitializers(const MachineDecl &machine) {
  for (const auto &decl : machine.vars) {
    AnalyzeVarDecl(decl);
  }
}

void MachineAnalyzer::AnalyzeVarDecl(const VarDecl &decl) {
  if (decl.initializer == nullptr) {
    return;
  }

  ValueType init_type = _expr_analyzer.Infer(*decl.initializer);
  if (init_type == ValueType::Invalid) {
    return;
  }

  // Reuse assignment rules by checking whether the initializer can flow into the declared type.
  const auto is_numeric = [](ValueType type) {
    switch (type) {
    case ValueType::Char:
    case ValueType::Short:
    case ValueType::Int:
    case ValueType::Float:
    case ValueType::Double:
      return true;
    default:
      return false;
    }
  };

  const auto numeric_rank = [](ValueType type) {
    switch (type) {
    case ValueType::Char:
      return 0;
    case ValueType::Short:
      return 1;
    case ValueType::Int:
      return 2;
    case ValueType::Float:
      return 3;
    case ValueType::Double:
      return 4;
    default:
      return -1;
    }
  };

  bool assignable = decl.type == init_type;
  if (!assignable && is_numeric(decl.type) && is_numeric(init_type)) {
    assignable = numeric_rank(init_type) <= numeric_rank(decl.type);
  }

  if (!assignable) {
    _ctx.Error("initializer for variable '" + decl.name + "' is not assignable to its declared type");
  }
}

void MachineAnalyzer::AnalyzeTransition(const TransitionDecl &transition) {
  const MachineDecl *machine = _ctx.CurrentMachineDecl();
  if (machine == nullptr) {
    _ctx.Error("transition analysis requires an active machine");
    return;
  }

  if (!_ctx.HasState(machine->name, transition.src_state)) {
    _ctx.Error("source state '" + transition.src_state + "' is not declared in machine '" + machine->name + "'");
  }
  if (!_ctx.HasState(machine->name, transition.dst_state)) {
    _ctx.Error("destination state '" + transition.dst_state + "' is not declared in machine '" + machine->name + "'");
  }

  _ctx.PushScope();
  AnalyzeTrigger(transition.trigger);
  AnalyzeGuard(transition.guard.get());
  if (transition.actions != nullptr) {
    _stmt_analyzer.AnalyzeBlock(*transition.actions);
  }
  _ctx.PopScope();
}

void MachineAnalyzer::AnalyzeTrigger(const TransitionTrigger &trigger) {
  std::visit(
      [this](const auto &typed_trigger) {
        using TriggerType = std::decay_t<decltype(typed_trigger)>;
        if constexpr (std::is_same_v<TriggerType, OnEventTrigger>) {
          AnalyzeOnEventTrigger(typed_trigger);
        } else if constexpr (std::is_same_v<TriggerType, AfterTrigger>) {
          AnalyzeAfterTrigger(typed_trigger);
        }
      },
      trigger);
}

void MachineAnalyzer::AnalyzeOnEventTrigger(const OnEventTrigger &trigger) {
  const MachineDecl *machine = _ctx.CurrentMachineDecl();
  if (machine == nullptr) {
    _ctx.Error("event trigger analysis requires an active machine");
    return;
  }

  const EventInfo *event = _ctx.FindEvent(machine->name, trigger.event_name);
  if (event == nullptr || event->decl == nullptr) {
    _ctx.Error("event '" + trigger.event_name + "' is not declared in machine '" + machine->name + "'");
    return;
  }
  _semantic_model.RecordOnEventResolution(trigger, *event->decl);

  const auto &params = event->decl->params;
  if (trigger.bindings.size() != params.size()) {
    _ctx.Error("event '" + trigger.event_name + "' expects " + std::to_string(params.size()) + " binding(s), got " +
               std::to_string(trigger.bindings.size()));
  }

  const size_t count = std::min(trigger.bindings.size(), params.size());
  for (size_t i = 0; i < count; ++i) {
    _ctx.DeclareTriggerBindingSymbol(params[i], trigger.bindings[i]);
  }
}

void MachineAnalyzer::AnalyzeAfterTrigger(const AfterTrigger &trigger) {
  if (trigger.delay.duration <= 0) {
    _ctx.Error("after trigger delay must be positive");
  }

  if (trigger.retry.has_value() && *trigger.retry <= 0) {
    _ctx.Error("after trigger retry delay must be positive");
  }
}

void MachineAnalyzer::AnalyzeGuard(const Expr *guard) {
  if (guard == nullptr) {
    return;
  }

  ValueType guard_type = _expr_analyzer.Infer(*guard);
  if (guard_type == ValueType::Invalid) {
    return;
  }

  if (guard_type != ValueType::Bool) {
    _ctx.Error("transition guard must have type 'bool'");
  }
}

} // namespace flux
