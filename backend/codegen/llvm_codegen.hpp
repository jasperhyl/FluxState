#pragma once
#include <string>

namespace llvm {
class Module;
}

namespace flux {

bool EmitObjectFile(llvm::Module &module, const std::string &output_path, std::string *error = nullptr);

} // namespace flux
