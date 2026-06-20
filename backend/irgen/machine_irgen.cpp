#include "machine_irgen.hpp"
#include "expr_irgen.hpp"
#include "irgen_context.hpp"
#include "stmt_irgen.hpp"
#include "type_converter.hpp"
#include "value_utils.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

namespace flux {
namespace {

// index是machine.transitions对应的索引
std::string GuardName(const LoweredMachine &machine, size_t index) {
  return "fs_guard_" + machine.name + "_" + std::to_string(index);
}

// 这里的index也是
std::string ActionName(const LoweredMachine &machine, size_t index) {
  return "fs_action_" + machine.name + "_" + std::to_string(index);
}

std::string InitName(const LoweredMachine &machine) { return "fs_init_" + machine.name; }

std::string DispatchName(const LoweredMachine &machine) { return "fs_dispatch_" + machine.name; }

std::string GlobalMachineName(const LoweredMachine &machine) { return "fs_machine_" + machine.name; }

// 时间单位转换
int64_t ToNanoseconds(const LoweredTimerSpec &timer) {
  switch (timer.unit) {
  case DurationLiteral::Unit::MilliSeconds:
    return timer.delay * 1000LL * 1000LL;
  case DurationLiteral::Unit::Minutes:
    return timer.delay * 60LL * 1000LL * 1000LL * 1000LL;
  case DurationLiteral::Unit::Seconds:
  default:
    return timer.delay * 1000LL * 1000LL * 1000LL;
  }
}

llvm::Value *NormalizeBool(IRGenContext &ctx, llvm::Value *value) {
  // 如果这个值本来就已经是 i1，也就是 LLVM 里的标准布尔类型,那就直接返回
  if (value->getType()->isIntegerTy(1)) {
    return value;
  }
  // 如果不是i1，但仍然是整数类型，就生成一个不等于0的比较
  if (value->getType()->isIntegerTy()) {
    return ctx.Builder().CreateICmpNE(value, llvm::ConstantInt::get(value->getType(), 0));
  }
  // 否则原样返回
  return value;
}

} // namespace

MachineIRGen::MachineIRGen(IRGenContext &ctx, TypeConverter &types, ExprIRGen &expr_irgen, StmtIRGen &stmt_irgen)
    : _ctx(ctx), _types(types), _expr_irgen(expr_irgen), _stmt_irgen(stmt_irgen) {}

void MachineIRGen::PrepareMachine(const LoweredMachine &machine) {
  MachineIRLayout layout = BuildLayout(machine);
  auto *global = new llvm::GlobalVariable(_ctx.Module(), layout.machine_type, false, llvm::GlobalValue::InternalLinkage,
                                          BuildMachineInitializer(machine, layout), GlobalMachineName(machine));
  _ctx.RegisterMachineInstance(machine.name, MachineInstanceIRInfo{
                                                 .layout = std::move(layout),
                                                 .global = global,
                                             });
}

void MachineIRGen::EmitMachine(const LoweredMachine &machine) {
  const auto *instance = _ctx.FindMachineInstance(machine.name);
  if (instance == nullptr) {
    _ctx.Error("missing prepared machine instance for '" + machine.name + "'");
    return;
  }
  const MachineIRLayout &layout = instance->layout;
  EmitGuardFunctions(machine, layout);
  EmitActionFunctions(machine, layout);
  EmitDispatchFunction(machine, layout);
  EmitInitFunction(machine, layout, instance->global);
}

MachineIRLayout MachineIRGen::BuildLayout(const LoweredMachine &machine) {
  MachineIRLayout layout;
  layout.machine = &machine;
  layout.header_type = _types.GetMachineHeaderType();

  std::vector<llvm::Type *> env_field_types;
  for (size_t i = 0; i < machine.env.fields.size(); ++i) {
    env_field_types.push_back(_types.ToIRType(machine.env.fields[i].type, true));
    layout.env_field_indices.emplace(machine.env.fields[i].name, i);
  }
  layout.env_type = llvm::StructType::create(_ctx.LLVM(), machine.name + ".env");
  layout.env_type->setBody(env_field_types);

  layout.machine_type = llvm::StructType::create(_ctx.LLVM(), machine.name + ".machine");
  layout.machine_type->setBody({layout.header_type, layout.env_type});

  for (const auto &event : machine.events) {
    layout.events_by_tag.emplace(event.tag, &event);
    if (!event.params.empty()) {
      layout.payload_types_by_event_tag.emplace(
          event.tag, _types.CreatePayloadType(machine.name + ".event." + event.name + ".payload", event));
    }
  }

  return layout;
}

llvm::Constant *MachineIRGen::BuildMachineInitializer(const LoweredMachine &machine, const MachineIRLayout &layout) {
  auto &llvm_ctx = _ctx.LLVM();
  llvm::Constant *header_init = llvm::ConstantStruct::get(
      layout.header_type, {
                              llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm_ctx), machine.initial_state_tag),
                              llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm_ctx), 0),
                              llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm_ctx), 0),
                              llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm_ctx, 0)),
                              llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm_ctx, 0)),
                              llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm_ctx, 0)),
                          });

  llvm::Constant *env_init = llvm::ConstantAggregateZero::get(layout.env_type);
  return llvm::ConstantStruct::get(layout.machine_type, {header_init, env_init});
}

void MachineIRGen::EmitInitFunction(const LoweredMachine &machine, const MachineIRLayout &layout,
                                    llvm::GlobalVariable *global) {
  auto *function = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(_ctx.LLVM()), false),
                                          llvm::Function::ExternalLinkage, InitName(machine), _ctx.Module());
  auto *entry = llvm::BasicBlock::Create(_ctx.LLVM(), "entry", function);
  auto *initialize = llvm::BasicBlock::Create(_ctx.LLVM(), "initialize", function);
  auto *done = llvm::BasicBlock::Create(_ctx.LLVM(), "done", function);
  _ctx.Builder().SetInsertPoint(entry);

  llvm::Value *header_ptr = _ctx.Builder().CreateStructGEP(layout.machine_type, global, 0, machine.name + ".hdr");
  llvm::Function *dispatch = _ctx.Module().getFunction(DispatchName(machine));
  llvm::Value *did_init = _ctx.Builder().CreateCall(
      _ctx.Runtime().init_machine,
      {header_ptr, llvm::ConstantInt::get(llvm::Type::getInt32Ty(_ctx.LLVM()), machine.initial_state_tag), dispatch});
  _ctx.Builder().CreateCondBr(did_init, initialize, done);

  _ctx.Builder().SetInsertPoint(initialize);
  _ctx.SetCurrentFunction(function);
  _ctx.SetCurrentMachineValue(global);
  _ctx.SetCurrentEventValue(nullptr);
  _ctx.SetCurrentMachineLayout(layout);
  _ctx.PushValueScope();

  llvm::Value *env_ptr = _ctx.Builder().CreateStructGEP(layout.machine_type, global, 1, machine.name + ".env");
  for (size_t i = 0; i < machine.env.initializers.size(); ++i) {
    if (!machine.env.initializers[i]) {
      continue;
    }
    llvm::Value *field_ptr =
        _ctx.Builder().CreateStructGEP(layout.env_type, env_ptr, i, machine.env.fields[i].name + ".init");
    llvm::Value *value = _expr_irgen.EmitExpr(*machine.env.initializers[i]);
    _ctx.Builder().CreateStore(
        AdjustForStore(_ctx, _types, machine.env.fields[i].type, machine.env.initializers[i]->type, value), field_ptr);
  }

  EmitStartTimersForState(machine, machine.initial_state_tag, header_ptr);
  _ctx.PopValueScope();
  _ctx.Builder().CreateBr(done);

  _ctx.Builder().SetInsertPoint(done);
  _ctx.Builder().CreateRetVoid();
}

void MachineIRGen::EmitGuardFunctions(const LoweredMachine &machine, const MachineIRLayout &layout) {
  // i1是指bool
  // llvm::FunctionType::get(返回类型, 参数类型列表, 是否可变参数)
  auto *function_type =
      llvm::FunctionType::get(llvm::Type::getInt1Ty(_ctx.LLVM()),
                              {llvm::PointerType::get(_ctx.LLVM(), 0), llvm::PointerType::get(_ctx.LLVM(), 0)}, false);

  for (size_t i = 0; i < machine.transitions.size(); ++i) {
    const auto &transition = machine.transitions[i];
    if (!transition.guard) {
      continue;
    }

    llvm::Function *function =
        llvm::Function::Create(function_type, llvm::Function::InternalLinkage, GuardName(machine, i), _ctx.Module());
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(_ctx.LLVM(), "entry", function);
    _ctx.Builder().SetInsertPoint(entry);
    _ctx.SetCurrentFunction(function);
    _ctx.SetCurrentMachineValue(function->getArg(0));
    _ctx.SetCurrentEventValue(function->getArg(1));
    _ctx.SetCurrentMachineLayout(layout);
    _ctx.PushValueScope();
    BindTransitionPayload(transition, layout, function);
    llvm::Value *guard_value = _expr_irgen.EmitExpr(*transition.guard);
    _ctx.Builder().CreateRet(NormalizeBool(_ctx, guard_value));
    _ctx.PopValueScope();
  }
}

void MachineIRGen::EmitActionFunctions(const LoweredMachine &machine, const MachineIRLayout &layout) {
  auto *function_type =
      llvm::FunctionType::get(llvm::Type::getVoidTy(_ctx.LLVM()),
                              {llvm::PointerType::get(_ctx.LLVM(), 0), llvm::PointerType::get(_ctx.LLVM(), 0)}, false);

  for (size_t i = 0; i < machine.transitions.size(); ++i) {
    const auto &transition = machine.transitions[i];
    if (!transition.action) {
      continue;
    }

    llvm::Function *function =
        llvm::Function::Create(function_type, llvm::Function::InternalLinkage, ActionName(machine, i), _ctx.Module());
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(_ctx.LLVM(), "entry", function);
    _ctx.Builder().SetInsertPoint(entry);
    _ctx.SetCurrentFunction(function);
    _ctx.SetCurrentMachineValue(function->getArg(0));
    _ctx.SetCurrentEventValue(function->getArg(1));
    _ctx.SetCurrentMachineLayout(layout);
    _ctx.PushValueScope();
    BindTransitionPayload(transition, layout, function);
    _stmt_irgen.EmitStmt(*transition.action); // 这个是关键
    _ctx.Builder().CreateRetVoid();
    _ctx.PopValueScope();
  }
}

// 为某个machine生成它的“事件分发函数”
// 也就是运行时收到一个事件后，如何根据：当前状态 current_state,当前事件 event.tag,各条 transition 的 guard,来决定走哪条
// transition，并执行对应迁移。
/*
void dispatch(machine, event) {
  switch (machine.current_state) {
    case StateA:
      switch (event.tag) {
        case Event1:
          if (guard1(machine, event)) {
            execute_transition_1(machine, event);
            return;
          }
          if (guard2(machine, event)) {
            execute_transition_2(machine, event);
            return;
          }
          return;
      }
      return;

    case StateB:
      ...
  }

  return;
}
先看当前处于哪个状态
再看当前事件是什么
找到这个状态下、由这个事件触发的所有 transition
按顺序检查 guard
命中一条就执行 transition 并返回
都不命中就直接返回
*/
void MachineIRGen::EmitDispatchFunction(const LoweredMachine &machine, const MachineIRLayout &layout) {
  // 生成void fs_dispatch_xxx(void* machine, void* event)
  auto *function_type =
      llvm::FunctionType::get(llvm::Type::getVoidTy(_ctx.LLVM()),
                              {llvm::PointerType::get(_ctx.LLVM(), 0), llvm::PointerType::get(_ctx.LLVM(), 0)}, false);
  llvm::Function *function =
      llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, DispatchName(machine), _ctx.Module());

  llvm::BasicBlock *entry = llvm::BasicBlock::Create(_ctx.LLVM(), "entry", function);
  llvm::BasicBlock *return_block = llvm::BasicBlock::Create(_ctx.LLVM(), "dispatch.return", function);
  _ctx.Builder().SetInsertPoint(entry);
  _ctx.SetCurrentFunction(function);
  _ctx.SetCurrentMachineValue(function->getArg(0));
  _ctx.SetCurrentEventValue(function->getArg(1));
  _ctx.SetCurrentMachineLayout(layout);

  llvm::Value *machine_value = function->getArg(0);
  llvm::Value *event_value = function->getArg(1);
  llvm::Value *header_ptr = _ctx.Builder().CreateStructGEP(layout.machine_type, machine_value, 0, "hdr");
  llvm::Value *state_ptr = _ctx.Builder().CreateStructGEP(layout.header_type, header_ptr, 0, "current_state.addr");
  llvm::Value *event_tag_ptr = _ctx.Builder().CreateStructGEP(_types.GetEventType(), event_value, 0, "event.tag.addr");
  llvm::Value *current_state = _ctx.Builder().CreateLoad(llvm::Type::getInt32Ty(_ctx.LLVM()), state_ptr);
  llvm::Value *event_tag = _ctx.Builder().CreateLoad(llvm::Type::getInt32Ty(_ctx.LLVM()), event_tag_ptr);

  llvm::SwitchInst *state_switch = _ctx.Builder().CreateSwitch(current_state, return_block, machine.states.size());

  for (const auto &state : machine.states) {
    llvm::BasicBlock *state_block = llvm::BasicBlock::Create(_ctx.LLVM(), "state." + state.name, function);
    state_switch->addCase(llvm::ConstantInt::get(llvm::Type::getInt32Ty(_ctx.LLVM()), state.tag), state_block);

    std::vector<size_t> state_transitions;
    for (size_t i = 0; i < machine.transitions.size(); ++i) {
      if (machine.transitions[i].src_state_tag == state.tag) {
        state_transitions.push_back(i);
      }
    }

    _ctx.Builder().SetInsertPoint(state_block);
    llvm::SwitchInst *event_switch =
        _ctx.Builder().CreateSwitch(event_tag, return_block, static_cast<unsigned>(state_transitions.size()));

    std::unordered_map<int32_t, std::vector<size_t>> grouped;
    for (size_t transition_index : state_transitions) {
      grouped[machine.transitions[transition_index].trigger_id].push_back(transition_index);
    }

    for (const auto &[trigger_id, transition_indices] : grouped) {
      llvm::BasicBlock *case_block =
          llvm::BasicBlock::Create(_ctx.LLVM(), "trigger." + std::to_string(trigger_id), function);
      event_switch->addCase(llvm::ConstantInt::get(llvm::Type::getInt32Ty(_ctx.LLVM()), trigger_id), case_block);

      llvm::BasicBlock *current_test_block = case_block;
      for (size_t i = 0; i < transition_indices.size(); ++i) {
        const size_t transition_index = transition_indices[i];
        const auto &transition = machine.transitions[transition_index];
        llvm::BasicBlock *execute_block =
            llvm::BasicBlock::Create(_ctx.LLVM(), "transition.exec." + std::to_string(transition_index), function);
        llvm::BasicBlock *next_block =
            (i + 1 < transition_indices.size())
                ? llvm::BasicBlock::Create(_ctx.LLVM(), "transition.next." + std::to_string(transition_index), function)
                : return_block;

        _ctx.Builder().SetInsertPoint(current_test_block);
        if (transition.guard) {
          llvm::Function *guard_function = _ctx.Module().getFunction(GuardName(machine, transition_index));
          llvm::Value *guard_value = _ctx.Builder().CreateCall(guard_function, {machine_value, event_value});
          _ctx.Builder().CreateCondBr(guard_value, execute_block, next_block);
        } else {
          _ctx.Builder().CreateBr(execute_block);
        }

        _ctx.Builder().SetInsertPoint(execute_block);
        EmitExecuteTransition(machine, layout, transition, transition_index);
        _ctx.Builder().CreateRetVoid();

        if (transition.guard && i + 1 < transition_indices.size()) {
          current_test_block = next_block;
        }
      }
    }
  }

  _ctx.Builder().SetInsertPoint(return_block);
  _ctx.Builder().CreateRetVoid();
}

void MachineIRGen::BindTransitionPayload(const LoweredTransition &transition, const MachineIRLayout &layout,
                                         llvm::Function *function) {
  if (transition.bindings.empty()) {
    return;
  }

  auto payload_it = layout.payload_types_by_event_tag.find(transition.trigger_id);
  if (payload_it == layout.payload_types_by_event_tag.end()) {
    return;
  }

  llvm::Value *event_arg = function->getArg(1); // 第二个参数，也就是event
  // 就是FS_Event的void* payload,但是是void**。event_arg其实就是FS_Event*
  llvm::Value *payload_ptr = _ctx.Builder().CreateStructGEP(_types.GetEventType(), event_arg, 4, "event.payload.addr");
  // payload_value = event->payload;真正拿到payload指针
  llvm::Value *payload_value =
      _ctx.Builder().CreateLoad(llvm::PointerType::get(_ctx.LLVM(), 0), payload_ptr, "event.payload");
  // 把 payload_value 变成一个后续方便继续做字段访问的指针值。typed_payload = (SomePayloadStruct*) event->payload;
  llvm::Value *typed_payload =
      _ctx.Builder().CreateBitCast(payload_value, llvm::PointerType::get(_ctx.LLVM(), 0), "typed.payload");

  for (const auto &binding : transition.bindings) {
    // 告诉payload_it->second，即struct具体的type，才能正确解释type_payload
    llvm::Value *field_ptr = _ctx.Builder().CreateStructGEP(payload_it->second, typed_payload, binding.index);
    llvm::Value *loaded = _ctx.Builder().CreateLoad(_types.ToIRType(binding.type, true), field_ptr, binding.name);
    if (binding.type.kind == LoweredTypeKind::Bool && loaded->getType()->isIntegerTy(8)) {
      loaded = _ctx.Builder().CreateTrunc(loaded, _types.ToIRType(binding.type, false));
    }
    _ctx.BindValue(binding.name, loaded);
  }
}

// 某一条 transition 被选中之后，真正执行这次状态迁移本身。
void MachineIRGen::EmitExecuteTransition(const LoweredMachine &machine, const MachineIRLayout &layout,
                                         const LoweredTransition &transition, size_t transition_index) {
  llvm::Value *machine_value = _ctx.CurrentMachineValue();
  llvm::Value *event_value = _ctx.CurrentEventValue();
  // 等价auto *header_ptr = &machine->hdr;
  llvm::Value *header_ptr = _ctx.Builder().CreateStructGEP(layout.machine_type, machine_value, 0, "hdr");
  // 等价auto *state_ptr = &machine->hdr.current_state;
  llvm::Value *state_ptr = _ctx.Builder().CreateStructGEP(layout.header_type, header_ptr, 0, "state.addr");

  if (transition.action) {
    llvm::Function *action_function = _ctx.Module().getFunction(ActionName(machine, transition_index));
    _ctx.Builder().CreateCall(action_function, {machine_value, event_value});
  }

  if (_debug) {
    std::string src_name = "?";
    std::string dst_name = "?";
    for (const auto &state : machine.states) {
      if (state.tag == transition.src_state_tag) src_name = state.name;
      if (state.tag == transition.dst_state_tag) dst_name = state.name;
    }
    std::string msg = "[" + machine.name + "] " + src_name + " -> " + dst_name;
    llvm::Value *msg_ptr = _ctx.Builder().CreateGlobalString(msg, "dbg.tr");
    _ctx.Builder().CreateCall(_ctx.Runtime().puts_fn, {msg_ptr});
  }

  // 准备离开 src_state，那么这个状态下挂着的 after / timeout 定时器应该取消掉
  EmitCancelTimersForState(machine, transition.src_state_tag, header_ptr);
  // 把当前状态写成目标状态
  // CreateStore(要写入的值，写入的目标地址)
  _ctx.Builder().CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(_ctx.LLVM()), transition.dst_state_tag),
                             state_ptr);
  // 启动新状态相关 timer
  EmitStartTimersForState(machine, transition.dst_state_tag, header_ptr);
}

// 生成一条对 runtime 函数 fs_start_timer(...) 的调用 IR。
// 真正启动 timer 的动作发生在运行时，也就是 runtime/runtime.c 里的 fs_start_timer。
void MachineIRGen::EmitStartTimersForState(const LoweredMachine &machine, int32_t state_tag,
                                           llvm::Value *machine_header_ptr) {
  for (const auto &transition : machine.transitions) {
    // 找出当前这个 state 下、由 after 触发、并且带 timer 配置”的 transition
    if (transition.trigger_kind != LoweredTriggerKind::After || transition.src_state_tag != state_tag ||
        !transition.timer.has_value()) {
      continue;
    }

    const auto delay_ns = ToNanoseconds(*transition.timer);
    const auto retry_ns =
        transition.timer->retry.has_value() ? transition.timer->retry.value() * 1000LL * 1000LL * 1000LL : 0LL;
    // 在 LLVM IR 里插入一条函数调用指令
    // 调用的是runtime.c的fs_start_timer函数
    _ctx.Builder().CreateCall(
        _ctx.Runtime().start_timer,
        {machine_header_ptr,
         llvm::ConstantInt::get(llvm::Type::getInt32Ty(_ctx.LLVM()), transition.timer->timeout_event_tag),
         llvm::ConstantInt::get(llvm::Type::getInt64Ty(_ctx.LLVM()), delay_ns),
         llvm::ConstantInt::get(llvm::Type::getInt64Ty(_ctx.LLVM()), retry_ns)});
  }
}

void MachineIRGen::EmitCancelTimersForState(const LoweredMachine &machine, int32_t state_tag,
                                            llvm::Value *machine_header_ptr) {
  for (const auto &transition : machine.transitions) {
    if (transition.trigger_kind != LoweredTriggerKind::After || transition.src_state_tag != state_tag ||
        !transition.timer.has_value()) {
      continue;
    }

    _ctx.Builder().CreateCall(_ctx.Runtime().cancel_timer,
                              {machine_header_ptr, llvm::ConstantInt::get(llvm::Type::getInt32Ty(_ctx.LLVM()),
                                                                          transition.timer->timeout_event_tag)});
  }
}

} // namespace flux
