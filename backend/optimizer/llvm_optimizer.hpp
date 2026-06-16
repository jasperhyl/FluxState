#pragma once

namespace llvm {
class Module;
}

namespace flux {

enum class OptimizationLevel {
  O0,
  O1,
  O2,
  O3,
};

void OptimizeModule(llvm::Module &module, OptimizationLevel level = OptimizationLevel::O2);

} // namespace flux
