#pragma once
#include "../ast/stmt.hpp"
#include "context.hpp"
#include "semantic_model.hpp"

namespace flux {

class ExprAnalyzer;

class StmtAnalyzer {
public:
  StmtAnalyzer(Context &ctx, ExprAnalyzer &expr_analyzer, SemanticModel &semantic_model);

  // 只负责语句与 block，不处理 machine 级声明。
  void AnalyzeBlock(const BlockStmt &block);
  void AnalyzeStmt(const Stmt &stmt);
  void AnalyzeExprStmt(const ExprStmt &stmt);
  void AnalyzeEmitStmt(const EmitStmt &stmt);

private:
  void AnalyzeEventInvoke(const EmitStmt &stmt);
  bool IsAssignable(ValueType lhs, ValueType rhs) const;

private:
  Context &_ctx;
  ExprAnalyzer &_expr_analyzer;
  SemanticModel &_semantic_model;
};

} // namespace flux
