#pragma once
#include "lowered_model.hpp"
#include "../../frontend/ast/machine.hpp"
#include "../../frontend/sema/semantic_model.hpp"
#include <string>
#include <vector>

namespace flux {

struct LoweringResult {
  LoweredProgram program;
  std::vector<std::string> diagnostics;
  bool Ok() const { return diagnostics.empty(); }
};

class Lowering {
public:
  LoweringResult Lower(const Program &program, const SemanticModel &semantic_model);
};

} // namespace flux
