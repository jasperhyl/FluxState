#include "lowering.hpp"
#include "expr_lowerer.hpp"
#include "lowering_context.hpp"
#include "machine_lowerer.hpp"
#include "stmt_lowerer.hpp"

namespace flux {

LoweringResult Lowering::Lower(const Program &program, const SemanticModel &semantic_model) {
  // 构造上下文ctx对象,集中管理lowering 阶段的上下文，包括诊断信息，semantic_model，current machine等等
  LoweringContext ctx(semantic_model);

  // 构造对应的lowerer对象
  ExprLowerer expr_lowerer(ctx);
  StmtLowerer stmt_lowerer(ctx, expr_lowerer);
  MachineLowerer machine_lowerer(ctx, expr_lowerer, stmt_lowerer);

  LoweredProgram lowered_program; // 保存所有lowered machine

  // 对每个machien调用LowerMachine，并收集返回的lowered machine
  for (const auto &machine : program.machines) {
    lowered_program.machines.push_back(machine_lowerer.LowerMachine(machine));
  }

  return LoweringResult{
      .program = std::move(lowered_program),
      .diagnostics = ctx.TakeDiagnostics(),
  };
}

} // namespace flux
