#pragma once
#include "lowered_model.hpp"
#include "lowering_context.hpp"

namespace flux {

class ExprLowerer {
public:
  explicit ExprLowerer(LoweringContext &ctx);

  std::unique_ptr<LoweredExpr> LowerExpr(const Expr &expr);

private:
  std::unique_ptr<LoweredExpr> LowerBoolLiteral(const BoolLiteralExpr &expr);
  std::unique_ptr<LoweredExpr> LowerCharLiteral(const CharLiteralExpr &expr);
  std::unique_ptr<LoweredExpr> LowerIntLiteral(const IntLiteralExpr &expr);
  std::unique_ptr<LoweredExpr> LowerStringLiteral(const StringLiteralExpr &expr);
  std::unique_ptr<LoweredExpr> LowerVar(const VarExpr &expr);
  std::unique_ptr<LoweredExpr> LowerUnary(const UnaryExpr &expr);
  std::unique_ptr<LoweredExpr> LowerBinary(const BinaryExpr &expr);
  std::unique_ptr<LoweredExpr> LowerCall(const CallExpr &expr);
  std::unique_ptr<LoweredExpr> LowerAssign(const AssignExpr &expr);
  LoweredType ResolveType(const Expr &expr) const;

private:
  LoweringContext &_ctx;
};

} // namespace flux
