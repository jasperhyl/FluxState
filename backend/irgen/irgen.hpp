#pragma once
#include "../lowering/lowered_model.hpp"
#include <memory>
#include <string>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
} // namespace llvm

namespace flux {

struct IRGenResult {
  std::unique_ptr<llvm::LLVMContext> llvm_context; // 生成IR时的全局上下文
  // 一整个 LLVM IR 模块。里面装的是函数、全局变量、目标平台信息、数据布局、符号表等，基本就是“这次编译产出的 IR
  // 文件本体”。
  std::unique_ptr<llvm::Module> module;
  std::vector<std::string> diagnostics;
  bool Ok() const { return diagnostics.empty(); }
};

class IRGen {
public:
  IRGenResult Generate(const LoweredProgram &program, const std::string &module_name = "flux_module");
};

} // namespace flux
