#pragma once
#include "../lowering/lowered_model.hpp"
#include "irgen_context.hpp"

namespace llvm {
class Function;
class GlobalVariable;
class StructType;
} // namespace llvm

namespace flux {

class IRGenContext;
class TypeConverter;
class ExprIRGen;
class StmtIRGen;

class MachineIRGen {
public:
  MachineIRGen(IRGenContext &ctx, TypeConverter &types, ExprIRGen &expr_irgen, StmtIRGen &stmt_irgen);

  void PrepareMachine(const LoweredMachine &machine);
  void EmitMachine(const LoweredMachine &machine);

private:
  MachineIRLayout BuildLayout(const LoweredMachine &machine);
  // 生成这个 machine 全局实例的初始常量值。 本质上是在拼一个 ConstantStruct，里面包含初始 header 和初始 env。
  llvm::Constant *BuildMachineInitializer(const LoweredMachine &machine, const MachineIRLayout &layout);
  // 生成这个 machine 的初始化函数，也就是类似 fs_init_xxx 的入口。 它会调用 runtime 的 init_machine，初始化 env
  // 初值，并在初始状态下启动需要的 timer。
  void EmitInitFunction(const LoweredMachine &machine, const MachineIRLayout &layout, llvm::GlobalVariable *global);
  // 为所有带 guard 的 transition 生成 guard 函数。 这些函数通常接收 machine/event，返回布尔值，表示该 transition
  // 当前是否可走。
  void EmitGuardFunctions(const LoweredMachine &machine, const MachineIRLayout &layout);
  // 为所有带 action 的 transition 生成 action 函数。 这些函数负责执行迁移动作里的语句，比如赋值、emit 事件等。
  void EmitActionFunctions(const LoweredMachine &machine, const MachineIRLayout &layout);
  // 生成 machine 的核心 dispatch 函数。 它会根据当前 state 和 event tag 做分发，检查 guard，执行
  // action，再触发状态迁移。
  void EmitDispatchFunction(const LoweredMachine &machine, const MachineIRLayout &layout);
  // 把当前事件 payload 里的字段，按 transition 的绑定规则提取出来并绑定成可用变量。 这样 guard/action
  // 里就能直接按名字使用事件参数。
  void BindTransitionPayload(const LoweredTransition &transition, const MachineIRLayout &layout,
                             llvm::Function *function);
  // 生成“真正执行某条 transition”那段逻辑。 会做几件事：调用 action、取消旧 state 的 timer、更新当前状态、启动新 state
  // 的 timer。
  void EmitExecuteTransition(const LoweredMachine &machine, const MachineIRLayout &layout,
                             const LoweredTransition &transition, size_t transition_index);
  // 为某个 state 下所有 after 型 transition 生成启动 timer 的调用。 也就是进入这个 state 时，把相关超时器都挂上。
  void EmitStartTimersForState(const LoweredMachine &machine, int32_t state_tag, llvm::Value *machine_header_ptr);
  // 为某个 state 下所有 after 型 transition 生成取消 timer 的调用。 也就是离开这个 state 时，把原来属于这个 state
  // 的超时器停掉。
  void EmitCancelTimersForState(const LoweredMachine &machine, int32_t state_tag, llvm::Value *machine_header_ptr);

private:
  IRGenContext &_ctx;
  TypeConverter &_types;
  ExprIRGen &_expr_irgen;
  StmtIRGen &_stmt_irgen;
};

} // namespace flux
