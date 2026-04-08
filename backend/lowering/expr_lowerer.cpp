#include "expr_lowerer.hpp"

namespace flux {

ExprLowerer::ExprLowerer(LoweringContext &ctx) : _ctx(ctx) {}

std::unique_ptr<LoweredExpr> ExprLowerer::LowerExpr(const Expr &expr) {
  // 尝试把静态基类expr动态转化为BoolLiteralExpr，如果可以转化就调用对应函数lowering，如果不能就返回nullptr
  if (const auto *node = dynamic_cast<const BoolLiteralExpr *>(&expr)) {
    return LowerBoolLiteral(*node);
  }
  if (const auto *node = dynamic_cast<const CharLiteralExpr *>(&expr)) {
    return LowerCharLiteral(*node);
  }
  if (const auto *node = dynamic_cast<const IntLiteralExpr *>(&expr)) {
    return LowerIntLiteral(*node);
  }
  if (const auto *node = dynamic_cast<const StringLiteralExpr *>(&expr)) {
    return LowerStringLiteral(*node);
  }
  if (const auto *node = dynamic_cast<const VarExpr *>(&expr)) {
    return LowerVar(*node);
  }
  if (const auto *node = dynamic_cast<const UnaryExpr *>(&expr)) {
    return LowerUnary(*node);
  }
  if (const auto *node = dynamic_cast<const BinaryExpr *>(&expr)) {
    return LowerBinary(*node);
  }
  if (const auto *node = dynamic_cast<const CallExpr *>(&expr)) {
    return LowerCall(*node);
  }
  if (const auto *node = dynamic_cast<const AssignExpr *>(&expr)) {
    return LowerAssign(*node);
  }

  _ctx.Error("unknown expression during lowering");
  return nullptr;
}

std::unique_ptr<LoweredExpr> ExprLowerer::LowerBoolLiteral(const BoolLiteralExpr &expr) {
  auto lowered = std::make_unique<LoweredExpr>();
  lowered->kind = LoweredExprKind::BoolLiteral;
  lowered->type = ResolveType(expr);
  lowered->source = &expr;
  lowered->bool_value = expr.value;
  return lowered;
}

std::unique_ptr<LoweredExpr> ExprLowerer::LowerCharLiteral(const CharLiteralExpr &expr) {
  auto lowered = std::make_unique<LoweredExpr>();
  lowered->kind = LoweredExprKind::CharLiteral;
  lowered->type = ResolveType(expr);
  lowered->source = &expr;
  lowered->char_value = expr.value;
  return lowered;
}

std::unique_ptr<LoweredExpr> ExprLowerer::LowerIntLiteral(const IntLiteralExpr &expr) {
  auto lowered = std::make_unique<LoweredExpr>();
  lowered->kind = LoweredExprKind::IntLiteral;
  lowered->type = ResolveType(expr);
  lowered->source = &expr;
  lowered->int_value = expr.value;
  return lowered;
}

std::unique_ptr<LoweredExpr> ExprLowerer::LowerStringLiteral(const StringLiteralExpr &expr) {
  auto lowered = std::make_unique<LoweredExpr>();
  lowered->kind = LoweredExprKind::StringLiteral;
  lowered->type = ResolveType(expr);
  lowered->source = &expr;
  lowered->string_value = expr.value;
  return lowered;
}

std::unique_ptr<LoweredExpr> ExprLowerer::LowerVar(const VarExpr &expr) {
  auto lowered = std::make_unique<LoweredExpr>();
  lowered->kind = LoweredExprKind::Var;
  lowered->type = ResolveType(expr);
  lowered->source = &expr;

  if (const auto *binding = _ctx.Semantic().FindVarBinding(expr)) {
    lowered->symbol.semantic_symbol = *binding;
  } else {
    _ctx.Error("missing semantic binding for variable '" + expr.name + "'");
  }

  if (const auto *symbol = _ctx.LookupSymbol(expr.name)) {
    lowered->symbol = *symbol;
  } else {
    lowered->symbol.name = expr.name;
    lowered->symbol.type = lowered->type;
    _ctx.Error("missing lowered symbol slot for variable '" + expr.name + "'");
  }

  return lowered;
}

std::unique_ptr<LoweredExpr> ExprLowerer::LowerUnary(const UnaryExpr &expr) {
  auto lowered = std::make_unique<LoweredExpr>();
  lowered->kind = LoweredExprKind::Unary;
  lowered->type = ResolveType(expr);
  lowered->source = &expr;
  lowered->unary_op = expr.op;
  if (expr.operand) {
    lowered->operand = LowerExpr(*expr.operand);
  }
  return lowered;
}

std::unique_ptr<LoweredExpr> ExprLowerer::LowerBinary(const BinaryExpr &expr) {
  auto lowered = std::make_unique<LoweredExpr>();
  lowered->kind = LoweredExprKind::Binary;
  lowered->type = ResolveType(expr);
  lowered->source = &expr;
  lowered->binary_op = expr.op;
  if (expr.lhs) {
    lowered->lhs = LowerExpr(*expr.lhs);
  }
  if (expr.rhs) {
    lowered->rhs = LowerExpr(*expr.rhs);
  }
  return lowered;
}

std::unique_ptr<LoweredExpr> ExprLowerer::LowerCall(const CallExpr &expr) {
  auto lowered = std::make_unique<LoweredExpr>();
  lowered->kind = LoweredExprKind::Call;
  lowered->type = ResolveType(expr);
  lowered->source = &expr;
  if (expr.callee) {
    lowered->callee = LowerExpr(*expr.callee);
  }
  for (const auto &arg : expr.args) {
    if (arg) {
      lowered->args.push_back(LowerExpr(*arg));
    }
  }
  return lowered;
}

std::unique_ptr<LoweredExpr> ExprLowerer::LowerAssign(const AssignExpr &expr) {
  auto lowered = std::make_unique<LoweredExpr>();
  lowered->kind = LoweredExprKind::Assign;
  lowered->type = ResolveType(expr);
  lowered->source = &expr;
  if (expr.lhs) {
    lowered->lhs = LowerExpr(*expr.lhs);
  }
  if (expr.rhs) {
    lowered->rhs = LowerExpr(*expr.rhs);
  }
  return lowered;
}

LoweredType ExprLowerer::ResolveType(const Expr &expr) const {
  auto type = _ctx.Semantic().FindExprType(expr);
  if (!type.has_value()) {
    return LoweredType{.kind = LoweredTypeKind::Invalid};
  }
  return LoweredTypeFromValueType(*type);
}

} // namespace flux
