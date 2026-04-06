#pragma once
#include "expr_lowerer.hpp"
#include "lowered_model.hpp"
#include "lowering_context.hpp"

namespace flux {

class StmtLowerer {
public:
  StmtLowerer(LoweringContext &ctx, ExprLowerer &expr_lowerer);

  std::unique_ptr<LoweredStmt> LowerStmt(const Stmt &stmt);
  std::unique_ptr<LoweredStmt> LowerBlock(const BlockStmt &stmt);

private:
  std::unique_ptr<LoweredStmt> LowerExprStmt(const ExprStmt &stmt);
  std::unique_ptr<LoweredStmt> LowerEmitStmt(const EmitStmt &stmt);

private:
  LoweringContext &_ctx;
  ExprLowerer &_expr_lowerer;
};

} // namespace flux
