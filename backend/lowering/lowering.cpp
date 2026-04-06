#include "lowering.hpp"
#include "expr_lowerer.hpp"
#include "lowering_context.hpp"
#include "machine_lowerer.hpp"
#include "stmt_lowerer.hpp"

namespace flux {

LoweringResult Lowering::Lower(const Program &program, const SemanticModel &semantic_model) {
  LoweringContext ctx(semantic_model);
  ExprLowerer expr_lowerer(ctx);
  StmtLowerer stmt_lowerer(ctx, expr_lowerer);
  MachineLowerer machine_lowerer(ctx, expr_lowerer, stmt_lowerer);

  LoweredProgram lowered_program;
  for (const auto &machine : program.machines) {
    lowered_program.machines.push_back(machine_lowerer.LowerMachine(machine));
  }

  return LoweringResult{
      .program = std::move(lowered_program),
      .diagnostics = ctx.TakeDiagnostics(),
  };
}

} // namespace flux
