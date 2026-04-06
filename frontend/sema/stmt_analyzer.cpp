#include "stmt_analyzer.hpp"
#include "expr_analyzer.hpp"
#include <algorithm>

namespace flux {

namespace {

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

} // namespace

StmtAnalyzer::StmtAnalyzer(Context &ctx, ExprAnalyzer &expr_analyzer, SemanticModel &semantic_model)
    : _ctx(ctx), _expr_analyzer(expr_analyzer), _semantic_model(semantic_model) {}

void StmtAnalyzer::AnalyzeBlock(const BlockStmt &block) {
  _ctx.PushScope();
  for (const auto &stmt : block.stmts) {
    if (stmt) {
      AnalyzeStmt(*stmt);
    }
  }
  _ctx.PopScope();
}

void StmtAnalyzer::AnalyzeStmt(const Stmt &stmt) {
  if (const auto *expr_stmt = dynamic_cast<const ExprStmt *>(&stmt)) {
    AnalyzeExprStmt(*expr_stmt);
    return;
  }
  if (const auto *emit_stmt = dynamic_cast<const EmitStmt *>(&stmt)) {
    AnalyzeEmitStmt(*emit_stmt);
    return;
  }
  if (const auto *block_stmt = dynamic_cast<const BlockStmt *>(&stmt)) {
    AnalyzeBlock(*block_stmt);
    return;
  }

  _ctx.Error("unknown statement kind");
}

void StmtAnalyzer::AnalyzeExprStmt(const ExprStmt &stmt) {
  if (stmt.expr) {
    _expr_analyzer.Infer(*stmt.expr);
  } else {
    _ctx.Error("expression statement is missing an expression");
  }
}

void StmtAnalyzer::AnalyzeEmitStmt(const EmitStmt &stmt) {
  ValueType target_type = ValueType::Invalid;
  if (stmt.target) {
    target_type = _expr_analyzer.Infer(*stmt.target);
  } else {
    _ctx.Error("emit statement is missing a target");
  }

  if (target_type != ValueType::Invalid && target_type != ValueType::MachineRef) {
    _ctx.Error("emit target must have machine type");
  }

  AnalyzeEventInvoke(stmt);
}

void StmtAnalyzer::AnalyzeEventInvoke(const EmitStmt &stmt) {
  const EventInvoke &invoke = stmt.invoke;
  if (invoke.event_name.empty()) {
    _ctx.Error("emit statement is missing an event name");
  }

  const auto *target_var = stmt.target == nullptr ? nullptr : dynamic_cast<const VarExpr *>(stmt.target.get());
  if (target_var == nullptr) {
    _ctx.Error("emit target must be a machine identifier");
    return;
  }

  const ResolvedSymbol *target_symbol = _semantic_model.FindVarBinding(*target_var);
  if (target_symbol == nullptr || target_symbol->kind != SymbolKind::MachineName || target_symbol->machine_decl == nullptr) {
    _ctx.Error("emit target '" + target_var->name + "' is not a machine");
    return;
  }

  const MachineDecl *target_machine = target_symbol->machine_decl;
  const EventInfo *event = _ctx.FindEvent(target_machine->name, invoke.event_name);
  if (event == nullptr || event->decl == nullptr) {
    _ctx.Error("event '" + invoke.event_name + "' is not declared in machine '" + target_machine->name + "'");
  } else {
    _semantic_model.RecordEmitResolution(stmt, *target_machine, *event->decl);
    if (invoke.args.size() != event->decl->params.size()) {
      _ctx.Error("event '" + invoke.event_name + "' expects " + std::to_string(event->decl->params.size()) +
                 " argument(s), got " + std::to_string(invoke.args.size()));
    }
  }

  for (const auto &arg : invoke.args) {
    if (arg) {
      _expr_analyzer.Infer(*arg);
    } else {
      _ctx.Error("event argument is missing an expression");
    }
  }

  if (event == nullptr || event->decl == nullptr) {
    return;
  }

  const size_t count = std::min(invoke.args.size(), event->decl->params.size());
  for (size_t i = 0; i < count; ++i) {
    if (invoke.args[i] == nullptr) {
      continue;
    }
    auto arg_type = _semantic_model.FindExprType(*invoke.args[i]);
    if (!arg_type.has_value() || *arg_type == ValueType::Invalid) {
      continue;
    }
    if (!IsAssignable(event->decl->params[i].type, *arg_type)) {
      _ctx.Error("argument " + std::to_string(i) + " of event '" + invoke.event_name + "' is not assignable to the "
                 "declared parameter type");
    }
  }
}

bool StmtAnalyzer::IsAssignable(ValueType lhs, ValueType rhs) const {
  if (lhs == ValueType::Invalid || rhs == ValueType::Invalid) {
    return false;
  }
  if (lhs == rhs) {
    return true;
  }
  if (NumericRank(lhs) >= 0 && NumericRank(rhs) >= 0) {
    return NumericRank(rhs) <= NumericRank(lhs);
  }
  return false;
}

} // namespace flux
