#include "llvm_optimizer.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>

namespace flux {
namespace {

llvm::OptimizationLevel ToLLVMOptimizationLevel(OptimizationLevel level) {
  switch (level) {
  case OptimizationLevel::O0:
    return llvm::OptimizationLevel::O0;
  case OptimizationLevel::O1:
    return llvm::OptimizationLevel::O1;
  case OptimizationLevel::O2:
    return llvm::OptimizationLevel::O2;
  case OptimizationLevel::O3:
    return llvm::OptimizationLevel::O3;
  }
  return llvm::OptimizationLevel::O2;
}

} // namespace

void OptimizeModule(llvm::Module &module, OptimizationLevel level) {
  llvm::LoopAnalysisManager loop_analyses;
  llvm::FunctionAnalysisManager function_analyses;
  llvm::CGSCCAnalysisManager cgscc_analyses;
  llvm::ModuleAnalysisManager module_analyses;
  llvm::PassBuilder pass_builder;

  pass_builder.registerModuleAnalyses(module_analyses);
  pass_builder.registerCGSCCAnalyses(cgscc_analyses);
  pass_builder.registerFunctionAnalyses(function_analyses);
  pass_builder.registerLoopAnalyses(loop_analyses);
  pass_builder.crossRegisterProxies(loop_analyses, function_analyses, cgscc_analyses, module_analyses);

  llvm::ModulePassManager module_passes = pass_builder.buildPerModuleDefaultPipeline(ToLLVMOptimizationLevel(level));
  module_passes.run(module, module_analyses);
}

} // namespace flux
