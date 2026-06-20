#include "runtime_decls.hpp"
#include "irgen_context.hpp"
#include "type_converter.hpp"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

namespace flux {

RuntimeDecls DeclareRuntimeDecls(IRGenContext &ctx, TypeConverter &types) {
  (void)types;
  auto &module = ctx.Module();
  auto &llvm_ctx = ctx.LLVM();

  auto *machine_header_ptr = llvm::PointerType::get(llvm_ctx, 0);
  auto *event_ptr = llvm::PointerType::get(llvm_ctx, 0);
  auto *dispatch_fn_ptr = llvm::PointerType::get(llvm_ctx, 0);

  RuntimeDecls decls;
  decls.enqueue_event = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_ctx), {machine_header_ptr, event_ptr}, false),
      llvm::Function::ExternalLinkage, "fs_enqueue_event", module);

  decls.start_timer = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_ctx),
                              {machine_header_ptr, llvm::Type::getInt32Ty(llvm_ctx), llvm::Type::getInt64Ty(llvm_ctx),
                               llvm::Type::getInt64Ty(llvm_ctx)},
                              false),
      llvm::Function::ExternalLinkage, "fs_start_timer", module);

  decls.cancel_timer = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_ctx),
                              {machine_header_ptr, llvm::Type::getInt32Ty(llvm_ctx)}, false),
      llvm::Function::ExternalLinkage, "fs_cancel_timer", module);

  decls.init_machine = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getInt1Ty(llvm_ctx),
                              {machine_header_ptr, llvm::Type::getInt32Ty(llvm_ctx), dispatch_fn_ptr}, false),
      llvm::Function::ExternalLinkage, "fs_init_machine", module);

  decls.cleanup_machine = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_ctx), {machine_header_ptr}, false), llvm::Function::ExternalLinkage,
      "fs_cleanup_machine", module);

  decls.runtime_step = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getInt1Ty(llvm_ctx), {machine_header_ptr}, false), llvm::Function::ExternalLinkage,
      "fs_runtime_step", module);

  decls.runtime_run = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_ctx), {machine_header_ptr}, false), llvm::Function::ExternalLinkage,
      "fs_runtime_run", module);

  decls.stop_machine = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_ctx), {machine_header_ptr}, false), llvm::Function::ExternalLinkage,
      "fs_stop_machine", module);

  decls.malloc_fn = llvm::Function::Create(
      llvm::FunctionType::get(llvm::PointerType::get(llvm_ctx, 0), {llvm::Type::getInt64Ty(llvm_ctx)}, false),
      llvm::Function::ExternalLinkage, "malloc", module);

  decls.puts_fn = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getInt32Ty(llvm_ctx), {llvm::PointerType::get(llvm_ctx, 0)}, false),
      llvm::Function::ExternalLinkage, "puts", module);

  return decls;
}

} // namespace flux
