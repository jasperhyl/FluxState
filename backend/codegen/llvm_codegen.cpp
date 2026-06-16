#include "llvm_codegen.hpp"
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
#include <memory>
#include <system_error>

namespace flux {

bool EmitObjectFile(llvm::Module &module, const std::string &output_path, std::string *error) {
  if (output_path.empty()) {
    if (error != nullptr) {
      *error = "object output path is empty";
    }
    return false;
  }

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  llvm::Triple target_triple = module.getTargetTriple().empty()
                                   ? llvm::Triple(llvm::sys::getDefaultTargetTriple())
                                   : llvm::Triple(module.getTargetTriple());
  module.setTargetTriple(target_triple);

  std::string target_error;
  const llvm::Target *target = llvm::TargetRegistry::lookupTarget(target_triple, target_error);
  if (target == nullptr) {
    if (error != nullptr) {
      *error = target_error;
    }
    return false;
  }

  llvm::TargetOptions target_options;
  std::unique_ptr<llvm::TargetMachine> target_machine(
      target->createTargetMachine(target_triple, "generic", "", target_options, llvm::Reloc::PIC_));
  if (!target_machine) {
    if (error != nullptr) {
      *error = "failed to create target machine";
    }
    return false;
  }

  module.setDataLayout(target_machine->createDataLayout());

  std::error_code ec;
  llvm::raw_fd_ostream dest(output_path, ec, llvm::sys::fs::OF_None);
  if (ec) {
    if (error != nullptr) {
      *error = ec.message();
    }
    return false;
  }

  llvm::legacy::PassManager pass_manager;
  if (target_machine->addPassesToEmitFile(pass_manager, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
    if (error != nullptr) {
      *error = "target machine cannot emit an object file";
    }
    return false;
  }

  pass_manager.run(module);
  dest.flush();
  return true;
}

} // namespace flux
