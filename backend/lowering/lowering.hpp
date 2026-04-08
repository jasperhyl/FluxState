#pragma once
#include "../../frontend/ast/machine.hpp"
#include "../../frontend/sema/semantic_model.hpp"
#include "lowered_model.hpp"
#include <string>
#include <vector>

namespace flux {

// 最后的返回结果
struct LoweringResult {
  LoweredProgram program;
  std::vector<std::string> diagnostics;
  bool Ok() const { return diagnostics.empty(); }
};

// 对外主入口
class Lowering {
public:
  LoweringResult Lower(const Program &program, const SemanticModel &semantic_model);
};

} // namespace flux
