#include "expr_analyzer.hpp"
#include <string>

namespace flux {
namespace {

const char *TypeName(ValueType type) {
  switch (type) {
  case ValueType::Invalid:
    return "invalid";
  case ValueType::MachineRef:
    return "machine";
  case ValueType::Bool:
    return "bool";
  case ValueType::Char:
    return "char";
  case ValueType::Short:
    return "short";
  case ValueType::Int:
    return "int";
  case ValueType::Float:
    return "float";
  case ValueType::Double:
    return "double";
  case ValueType::String:
    return "string";
  }
  return "unknown";
}

const char *UnaryOpName(UnaryOp op) {
  switch (op) {
  case UnaryOp::Neg:
    return "-";
  case UnaryOp::Plus:
    return "+";
  case UnaryOp::AddressOf:
    return "&";
  case UnaryOp::Deref:
    return "*";
  }
  return "<unary-op>";
}

const char *BinaryOpName(BinaryOp op) {
  switch (op) {
  case BinaryOp::LogicAnd:
    return "&&";
  case BinaryOp::LogicOr:
    return "||";
  case BinaryOp::Add:
    return "+";
  case BinaryOp::Sub:
    return "-";
  case BinaryOp::Mul:
    return "*";
  case BinaryOp::Div:
    return "/";
  case BinaryOp::Eq:
    return "==";
  case BinaryOp::Ne:
    return "!=";
  case BinaryOp::Lt:
    return "<";
  case BinaryOp::Le:
    return "<=";
  case BinaryOp::Gt:
    return ">";
  case BinaryOp::Ge:
    return ">=";
  case BinaryOp::Assign:
    return "=";
  }
  return "<binary-op>";
}

int NumericRank(ValueType type) {
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
}

ValueType CommonNumericType(ValueType lhs, ValueType rhs) {
  return NumericRank(lhs) >= NumericRank(rhs) ? lhs : rhs;
}

} // namespace

ExprAnalyzer::ExprAnalyzer(Context &ctx, SemanticModel &semantic_model) : _ctx(ctx), _semantic_model(semantic_model) {}

ValueType ExprAnalyzer::Infer(const Expr &expr) {
  if (dynamic_cast<const BoolLiteralExpr *>(&expr) != nullptr) {
    return Remember(expr, ValueType::Bool);
  }
  if (dynamic_cast<const CharLiteralExpr *>(&expr) != nullptr) {
    return Remember(expr, ValueType::Char);
  }
  if (dynamic_cast<const IntLiteralExpr *>(&expr) != nullptr) {
    return Remember(expr, ValueType::Int);
  }
  if (dynamic_cast<const StringLiteralExpr *>(&expr) != nullptr) {
    return Remember(expr, ValueType::String);
  }

  if (const auto *var = dynamic_cast<const VarExpr *>(&expr)) {
    return Remember(expr, InferVar(*var));
  }
  if (const auto *unary = dynamic_cast<const UnaryExpr *>(&expr)) {
    return Remember(expr, InferUnary(*unary));
  }
  if (const auto *binary = dynamic_cast<const BinaryExpr *>(&expr)) {
    return Remember(expr, InferBinary(*binary));
  }
  if (const auto *call = dynamic_cast<const CallExpr *>(&expr)) {
    return Remember(expr, InferCall(*call));
  }
  if (const auto *assign = dynamic_cast<const AssignExpr *>(&expr)) {
    return Remember(expr, InferAssign(*assign));
  }

  _ctx.Error("unknown expression kind");
  return Remember(expr, ValueType::Invalid);
}

ValueType ExprAnalyzer::InferVar(const VarExpr &expr) {
  auto symbol = _ctx.ResolveSymbol(expr.name);
  if (!symbol.has_value()) {
    _ctx.Error("use of undeclared symbol '" + expr.name + "'");
    return ValueType::Invalid;
  }
  _semantic_model.RecordVarBinding(expr, *symbol);
  return symbol->type;
}

ValueType ExprAnalyzer::InferUnary(const UnaryExpr &expr) {
  ValueType operand_type = Infer(*expr.operand);
  if (operand_type == ValueType::Invalid) {
    return ValueType::Invalid;
  }

  switch (expr.op) {
  case UnaryOp::Neg:
  case UnaryOp::Plus:
    if (!IsNumeric(operand_type)) {
      _ctx.Error("operator '" + std::string(UnaryOpName(expr.op)) + "' requires a numeric operand, got '" +
                 TypeName(operand_type) + "'");
      return ValueType::Invalid;
    }
    return operand_type;
  case UnaryOp::AddressOf:
  case UnaryOp::Deref:
    _ctx.Error("operator '" + std::string(UnaryOpName(expr.op)) + "' is not supported yet");
    return ValueType::Invalid;
  }

  _ctx.Error("unknown unary operator");
  return ValueType::Invalid;
}

ValueType ExprAnalyzer::InferBinary(const BinaryExpr &expr) {
  ValueType lhs_type = Infer(*expr.lhs);
  ValueType rhs_type = Infer(*expr.rhs);
  if (lhs_type == ValueType::Invalid || rhs_type == ValueType::Invalid) {
    return ValueType::Invalid;
  }

  switch (expr.op) {
  case BinaryOp::LogicAnd:
  case BinaryOp::LogicOr:
    if (lhs_type != ValueType::Bool || rhs_type != ValueType::Bool) {
      _ctx.Error("operator '" + std::string(BinaryOpName(expr.op)) + "' requires bool operands, got '" +
                 TypeName(lhs_type) + "' and '" + TypeName(rhs_type) + "'");
      return ValueType::Invalid;
    }
    return ValueType::Bool;

  case BinaryOp::Add:
  case BinaryOp::Sub:
  case BinaryOp::Mul:
  case BinaryOp::Div:
    if (!IsNumeric(lhs_type) || !IsNumeric(rhs_type)) {
      _ctx.Error("operator '" + std::string(BinaryOpName(expr.op)) + "' requires numeric operands, got '" +
                 TypeName(lhs_type) + "' and '" + TypeName(rhs_type) + "'");
      return ValueType::Invalid;
    }
    return CommonNumericType(lhs_type, rhs_type);

  case BinaryOp::Eq:
  case BinaryOp::Ne:
    if (!IsComparable(lhs_type, rhs_type)) {
      _ctx.Error("operator '" + std::string(BinaryOpName(expr.op)) + "' cannot compare '" + TypeName(lhs_type) +
                 "' with '" + TypeName(rhs_type) + "'");
      return ValueType::Invalid;
    }
    return ValueType::Bool;

  case BinaryOp::Lt:
  case BinaryOp::Le:
  case BinaryOp::Gt:
  case BinaryOp::Ge:
    if (!IsNumeric(lhs_type) || !IsNumeric(rhs_type)) {
      _ctx.Error("operator '" + std::string(BinaryOpName(expr.op)) + "' requires numeric operands, got '" +
                 TypeName(lhs_type) + "' and '" + TypeName(rhs_type) + "'");
      return ValueType::Invalid;
    }
    return ValueType::Bool;

  case BinaryOp::Assign:
    _ctx.Error("binary operator '=' is not expected here");
    return ValueType::Invalid;
  }

  _ctx.Error("unknown binary operator");
  return ValueType::Invalid;
}

ValueType ExprAnalyzer::InferCall(const CallExpr &expr) {
  if (expr.callee) {
    Infer(*expr.callee);
  }
  for (const auto &arg : expr.args) {
    Infer(*arg);
  }

  _ctx.Error("call expression is not supported yet");
  return ValueType::Invalid;
}

ValueType ExprAnalyzer::InferAssign(const AssignExpr &expr) {
  const auto *lhs_var = dynamic_cast<const VarExpr *>(expr.lhs.get());
  if (lhs_var == nullptr) {
    if (expr.lhs) {
      Infer(*expr.lhs);
    }
    if (expr.rhs) {
      Infer(*expr.rhs);
    }
    _ctx.Error("left-hand side of assignment must be a variable");
    return ValueType::Invalid;
  }

  ValueType lhs_type = InferVar(*lhs_var);
  ValueType rhs_type = Infer(*expr.rhs);
  if (lhs_type == ValueType::Invalid || rhs_type == ValueType::Invalid) {
    return ValueType::Invalid;
  }

  if (!IsAssignable(lhs_type, rhs_type)) {
    _ctx.Error("cannot assign value of type '" + std::string(TypeName(rhs_type)) + "' to '" + TypeName(lhs_type) +
               "'");
    return ValueType::Invalid;
  }

  return lhs_type;
}

ValueType ExprAnalyzer::Remember(const Expr &expr, ValueType type) {
  _semantic_model.RecordExprType(expr, type);
  return type;
}

bool ExprAnalyzer::IsNumeric(ValueType type) const { return NumericRank(type) >= 0; }

bool ExprAnalyzer::IsComparable(ValueType lhs, ValueType rhs) const {
  if (lhs == ValueType::Invalid || rhs == ValueType::Invalid) {
    return false;
  }
  if (lhs == rhs) {
    return true;
  }
  return IsNumeric(lhs) && IsNumeric(rhs);
}

bool ExprAnalyzer::IsAssignable(ValueType lhs, ValueType rhs) const {
  if (lhs == ValueType::Invalid || rhs == ValueType::Invalid) {
    return false;
  }
  if (lhs == rhs) {
    return true;
  }
  if (IsNumeric(lhs) && IsNumeric(rhs)) {
    return NumericRank(rhs) <= NumericRank(lhs);
  }
  return false;
}

} // namespace flux
