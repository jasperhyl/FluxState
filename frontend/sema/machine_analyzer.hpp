#pragma once
#include "../ast/machine.hpp"
#include "context.hpp"
#include "semantic_model.hpp"

namespace flux {

class ExprAnalyzer;
class StmtAnalyzer;

class MachineAnalyzer {
public:
  MachineAnalyzer(Context &ctx, ExprAnalyzer &expr_analyzer, StmtAnalyzer &stmt_analyzer, SemanticModel &semantic_model);

  void AnalyzeMachine(const MachineDecl &machine);

private:
  void AnalyzeInitialState(const MachineDecl &machine);
  void DeclareMachineVars(const MachineDecl &machine);
  void AnalyzeMachineVarInitializers(const MachineDecl &machine);
  void AnalyzeVarDecl(const VarDecl &decl);

  void AnalyzeTransition(const TransitionDecl &transition);
  void AnalyzeTrigger(const TransitionTrigger &trigger);
  void AnalyzeOnEventTrigger(const OnEventTrigger &trigger);
  void AnalyzeAfterTrigger(const AfterTrigger &trigger);
  void AnalyzeGuard(const Expr *guard);

private:
  Context &_ctx;
  ExprAnalyzer &_expr_analyzer;
  StmtAnalyzer &_stmt_analyzer;
  SemanticModel &_semantic_model;
};

} // namespace flux
