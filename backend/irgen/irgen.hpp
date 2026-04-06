#pragma once
#include "../lowering/lowered_model.hpp"
#include <memory>
#include <string>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
}

namespace flux {

struct IRGenResult {
  std::unique_ptr<llvm::LLVMContext> llvm_context;
  std::unique_ptr<llvm::Module> module;
  std::vector<std::string> diagnostics;
  bool Ok() const { return diagnostics.empty(); }
};

class IRGen {
public:
  IRGenResult Generate(const LoweredProgram &program, const std::string &module_name = "flux_module");
};

} // namespace flux
