#pragma once
#include "../lowering/lowered_model.hpp"
#include "irgen_context.hpp"

namespace llvm {
class Function;
class GlobalVariable;
class StructType;
}

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
  llvm::Constant *BuildMachineInitializer(const LoweredMachine &machine, const MachineIRLayout &layout);
  void EmitInitFunction(const LoweredMachine &machine, const MachineIRLayout &layout, llvm::GlobalVariable *global);
  void EmitGuardFunctions(const LoweredMachine &machine, const MachineIRLayout &layout);
  void EmitActionFunctions(const LoweredMachine &machine, const MachineIRLayout &layout);
  void EmitDispatchFunction(const LoweredMachine &machine, const MachineIRLayout &layout);
  void BindTransitionPayload(const LoweredTransition &transition, const MachineIRLayout &layout, llvm::Function *function);
  void EmitExecuteTransition(const LoweredMachine &machine, const MachineIRLayout &layout, const LoweredTransition &transition,
                             size_t transition_index);
  void EmitStartTimersForState(const LoweredMachine &machine, int32_t state_tag, llvm::Value *machine_header_ptr);
  void EmitCancelTimersForState(const LoweredMachine &machine, int32_t state_tag, llvm::Value *machine_header_ptr);

private:
  IRGenContext &_ctx;
  TypeConverter &_types;
  ExprIRGen &_expr_irgen;
  StmtIRGen &_stmt_irgen;
};

} // namespace flux
