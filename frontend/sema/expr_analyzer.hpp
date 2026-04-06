#pragma once
#include "../ast/expr.hpp"
#include "context.hpp"
#include "semantic_model.hpp"

namespace flux {

class ExprAnalyzer {
public:
  ExprAnalyzer(Context &ctx, SemanticModel &semantic_model);

  // 仅分析 Expr 节点；变量声明与作用域建立由上层 analyzer 负责。
  // 若存在语义错误则返回 Invalid，并向 Context 写入诊断。
  ValueType Infer(const Expr &expr);

private:
  ValueType InferVar(const VarExpr &expr);
  ValueType InferUnary(const UnaryExpr &expr);
  ValueType InferBinary(const BinaryExpr &expr);
  ValueType InferCall(const CallExpr &expr);
  ValueType InferAssign(const AssignExpr &expr);
  ValueType Remember(const Expr &expr, ValueType type);

  bool IsNumeric(ValueType type) const;
  bool IsComparable(ValueType lhs, ValueType rhs) const;
  bool IsAssignable(ValueType lhs, ValueType rhs) const;

private:
  Context &_ctx;
  SemanticModel &_semantic_model;
};

} // namespace flux
