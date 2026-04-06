#include "stmt_lowerer.hpp"

namespace flux {
namespace {

int32_t FindDeclaredEventTag(const MachineDecl &machine, const std::string &event_name) {
  for (size_t i = 0; i < machine.events.size(); ++i) {
    if (machine.events[i].name == event_name) {
      return static_cast<int32_t>(i);
    }
  }
  return -1;
}

} // namespace

StmtLowerer::StmtLowerer(LoweringContext &ctx, ExprLowerer &expr_lowerer) : _ctx(ctx), _expr_lowerer(expr_lowerer) {}

std::unique_ptr<LoweredStmt> StmtLowerer::LowerStmt(const Stmt &stmt) {
  if (const auto *node = dynamic_cast<const ExprStmt *>(&stmt)) {
    return LowerExprStmt(*node);
  }
  if (const auto *node = dynamic_cast<const EmitStmt *>(&stmt)) {
    return LowerEmitStmt(*node);
  }
  if (const auto *node = dynamic_cast<const BlockStmt *>(&stmt)) {
    return LowerBlock(*node);
  }

  _ctx.Error("unknown statement during lowering");
  return nullptr;
}

std::unique_ptr<LoweredStmt> StmtLowerer::LowerBlock(const BlockStmt &stmt) {
  auto lowered = std::make_unique<LoweredStmt>();
  lowered->kind = LoweredStmtKind::Block;
  lowered->source = &stmt;

  _ctx.PushScope();
  for (const auto &child : stmt.stmts) {
    if (child) {
      lowered->stmts.push_back(LowerStmt(*child));
    }
  }
  _ctx.PopScope();

  return lowered;
}

std::unique_ptr<LoweredStmt> StmtLowerer::LowerExprStmt(const ExprStmt &stmt) {
  auto lowered = std::make_unique<LoweredStmt>();
  lowered->kind = LoweredStmtKind::Expr;
  lowered->source = &stmt;
  if (stmt.expr) {
    lowered->expr = _expr_lowerer.LowerExpr(*stmt.expr);
  }
  return lowered;
}

std::unique_ptr<LoweredStmt> StmtLowerer::LowerEmitStmt(const EmitStmt &stmt) {
  auto lowered = std::make_unique<LoweredStmt>();
  lowered->kind = LoweredStmtKind::Emit;
  lowered->source = &stmt;
  lowered->emit_event_name = stmt.invoke.event_name;

  if (stmt.target) {
    lowered->emit_target = _expr_lowerer.LowerExpr(*stmt.target);
  }

  if (const auto *target_machine = _ctx.Semantic().FindEmitTargetMachine(stmt)) {
    lowered->emit_target_machine_name = target_machine->name;
  }

  if (const auto *event = _ctx.Semantic().FindEmitResolution(stmt)) {
    if (const auto *target_machine = _ctx.Semantic().FindEmitTargetMachine(stmt)) {
      lowered->emit_event_tag = FindDeclaredEventTag(*target_machine, event->name);
    } else {
      lowered->emit_event_tag = _ctx.LookupEventTag(event->name);
    }
  } else if (!stmt.invoke.event_name.empty()) {
    lowered->emit_event_tag = _ctx.LookupEventTag(stmt.invoke.event_name);
  }

  if (lowered->emit_event_tag < 0 && !stmt.invoke.event_name.empty()) {
    _ctx.Error("missing lowered event tag for emit '" + stmt.invoke.event_name + "'");
  }

  for (const auto &arg : stmt.invoke.args) {
    if (arg) {
      lowered->emit_args.push_back(_expr_lowerer.LowerExpr(*arg));
    }
  }

  return lowered;
}

} // namespace flux
