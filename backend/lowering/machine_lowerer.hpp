#pragma once
#include "lowered_model.hpp"
#include "lowering_context.hpp"

namespace flux {

class ExprLowerer;
class StmtLowerer;

class MachineLowerer {
public:
  MachineLowerer(LoweringContext &ctx, ExprLowerer &expr_lowerer, StmtLowerer &stmt_lowerer);

  LoweredMachine LowerMachine(const MachineDecl &machine);

private:
  void LowerStates(const MachineDecl &machine, LoweredMachine &lowered_machine);
  void LowerEvents(const MachineDecl &machine, LoweredMachine &lowered_machine);
  void LowerEnv(const MachineDecl &machine, LoweredMachine &lowered_machine);
  void LowerTransitions(const MachineDecl &machine, LoweredMachine &lowered_machine);
  LoweredTransition LowerTransition(const TransitionDecl &transition, LoweredMachine &lowered_machine, size_t index);
  std::optional<LoweredTimerSpec> LowerAfterTimer(const TransitionDecl &transition, const AfterTrigger &trigger,
                                                  int32_t timeout_event_tag, std::string timeout_event_name);

private:
  LoweringContext &_ctx;
  ExprLowerer &_expr_lowerer;
  StmtLowerer &_stmt_lowerer;
};

} // namespace flux
