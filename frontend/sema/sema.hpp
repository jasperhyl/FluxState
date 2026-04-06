#pragma once
#include "../ast/machine.hpp"
#include "context.hpp"
#include "semantic_model.hpp"
#include <vector>

namespace flux {

struct SemaResult {
  std::vector<Diagnostic> diagnostics;
  SemanticModel semantic_model;
  bool Ok() const { return diagnostics.empty(); }
};

class Sema {
public:
  // 语义分析唯一入口。
  SemaResult Analyze(const Program &program);

private:
  // machine / state / event 符号表。
  void CollectMachines(const Program &program);
  void CaptureMachineInfos(SemanticModel &semantic_model) const;

private:
  Context _ctx;
};

} // namespace flux