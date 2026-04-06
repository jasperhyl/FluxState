#include "sema.hpp"
#include "expr_analyzer.hpp"
#include "machine_analyzer.hpp"
#include "stmt_analyzer.hpp"

namespace flux {

SemaResult Sema::Analyze(const Program &program) {
  _ctx = Context{};
  SemanticModel semantic_model;

  CollectMachines(program);
  CaptureMachineInfos(semantic_model);

  ExprAnalyzer expr_analyzer(_ctx, semantic_model);
  StmtAnalyzer stmt_analyzer(_ctx, expr_analyzer, semantic_model);
  MachineAnalyzer machine_analyzer(_ctx, expr_analyzer, stmt_analyzer, semantic_model);

  for (const auto &machine : program.machines) {
    machine_analyzer.AnalyzeMachine(machine);
  }

  return SemaResult{.diagnostics = _ctx.TakeDiagnostics(), .semantic_model = std::move(semantic_model)};
}

void Sema::CollectMachines(const Program &program) {
  for (const auto &machine : program.machines) {
    _ctx.DeclareMachine(machine);
  }
}

void Sema::CaptureMachineInfos(SemanticModel &semantic_model) const {
  for (const auto &[name, info] : _ctx.Machines()) {
    MachineSemanticInfo machine_info;
    machine_info.decl = info.decl;
    machine_info.states = info.states;
    for (const auto &[event_name, event_info] : info.events) {
      machine_info.events.emplace(event_name, event_info.decl);
    }
    semantic_model.RecordMachineInfo(name, std::move(machine_info));
  }
}

} // namespace flux
