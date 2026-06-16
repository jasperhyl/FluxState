#include "irgen.hpp"
#include "expr_irgen.hpp"
#include "irgen_context.hpp"
#include "machine_irgen.hpp"
#include "runtime_decls.hpp"
#include "stmt_irgen.hpp"
#include "type_converter.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/TargetParser/Triple.h>

namespace flux {
namespace {

std::string ModuleInitName() { return "fs_module_init"; }
std::string ModuleCleanupName() { return "fs_module_cleanup"; }
std::string InitMachineName(const LoweredMachine &machine) { return "fs_init_" + machine.name; }
std::string RunMachineName(const LoweredMachine &machine) { return "fs_run_" + machine.name; }
std::string StepMachineName(const LoweredMachine &machine) { return "fs_step_" + machine.name; }
std::string StopMachineName(const LoweredMachine &machine) { return "fs_stop_" + machine.name; }
std::string GetMachineName(const LoweredMachine &machine) { return "fs_get_machine_" + machine.name; }

llvm::Value *GetHeaderPtr(IRGenContext &ctx, const MachineInstanceIRInfo &instance) {
  return ctx.Builder().CreateStructGEP(instance.layout.machine_type, instance.global, 0, instance.layout.machine->name + ".hdr");
}

void EmitModuleInit(IRGenContext &ctx, const LoweredProgram &program) {
  auto *fn = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(ctx.LLVM()), false), llvm::Function::ExternalLinkage, ModuleInitName(),
      ctx.Module());
  auto *entry = llvm::BasicBlock::Create(ctx.LLVM(), "entry", fn);
  ctx.Builder().SetInsertPoint(entry);
  for (const auto &machine : program.machines) {
    llvm::Function *init_fn = ctx.Module().getFunction(InitMachineName(machine));
    if (init_fn == nullptr) {
      ctx.Error("missing machine init function while emitting module init for '" + machine.name + "'");
      continue;
    }
    ctx.Builder().CreateCall(init_fn);
  }
  ctx.Builder().CreateRetVoid();
}

void EmitModuleCleanup(IRGenContext &ctx, const LoweredProgram &program) {
  auto *fn = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(ctx.LLVM()), false), llvm::Function::ExternalLinkage, ModuleCleanupName(),
      ctx.Module());
  auto *entry = llvm::BasicBlock::Create(ctx.LLVM(), "entry", fn);
  ctx.Builder().SetInsertPoint(entry);
  for (const auto &machine : program.machines) {
    const auto *instance = ctx.FindMachineInstance(machine.name);
    if (instance == nullptr) {
      ctx.Error("missing machine instance while emitting module cleanup for '" + machine.name + "'");
      continue;
    }
    ctx.Builder().CreateCall(ctx.Runtime().cleanup_machine, {GetHeaderPtr(ctx, *instance)});
  }
  ctx.Builder().CreateRetVoid();
}

void EmitMachineWrappers(IRGenContext &ctx, const LoweredMachine &machine) {
  const auto *instance = ctx.FindMachineInstance(machine.name);
  if (instance == nullptr) {
    ctx.Error("missing machine instance while emitting wrappers for '" + machine.name + "'");
    return;
  }

  auto *machine_ptr_type = llvm::PointerType::get(ctx.LLVM(), 0);

  auto *get_fn = llvm::Function::Create(llvm::FunctionType::get(machine_ptr_type, false), llvm::Function::ExternalLinkage,
                                        GetMachineName(machine), ctx.Module());
  auto *get_entry = llvm::BasicBlock::Create(ctx.LLVM(), "entry", get_fn);
  ctx.Builder().SetInsertPoint(get_entry);
  ctx.Builder().CreateRet(instance->global);

  auto *run_fn = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(ctx.LLVM()), false),
                                        llvm::Function::ExternalLinkage, RunMachineName(machine), ctx.Module());
  auto *run_entry = llvm::BasicBlock::Create(ctx.LLVM(), "entry", run_fn);
  ctx.Builder().SetInsertPoint(run_entry);
  ctx.Builder().CreateCall(ctx.Module().getFunction(ModuleInitName()));
  ctx.Builder().CreateCall(ctx.Runtime().runtime_run, {GetHeaderPtr(ctx, *instance)});
  ctx.Builder().CreateRetVoid();

  auto *step_fn = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt1Ty(ctx.LLVM()), false),
                                         llvm::Function::ExternalLinkage, StepMachineName(machine), ctx.Module());
  auto *step_entry = llvm::BasicBlock::Create(ctx.LLVM(), "entry", step_fn);
  ctx.Builder().SetInsertPoint(step_entry);
  ctx.Builder().CreateCall(ctx.Module().getFunction(ModuleInitName()));
  ctx.Builder().CreateRet(ctx.Builder().CreateCall(ctx.Runtime().runtime_step, {GetHeaderPtr(ctx, *instance)}));

  auto *stop_fn = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(ctx.LLVM()), false),
                                         llvm::Function::ExternalLinkage, StopMachineName(machine), ctx.Module());
  auto *stop_entry = llvm::BasicBlock::Create(ctx.LLVM(), "entry", stop_fn);
  ctx.Builder().SetInsertPoint(stop_entry);
  ctx.Builder().CreateCall(ctx.Runtime().stop_machine, {GetHeaderPtr(ctx, *instance)});
  ctx.Builder().CreateRetVoid();
}

} // namespace

IRGenResult IRGen::Generate(const LoweredProgram &program, const std::string &module_name) {
  IRGenContext ctx(module_name);
  TypeConverter types(ctx);
  ctx.SetRuntimeDecls(DeclareRuntimeDecls(ctx, types));

  ExprIRGen expr_irgen(ctx, types);
  StmtIRGen stmt_irgen(ctx, types, expr_irgen);
  MachineIRGen machine_irgen(ctx, types, expr_irgen, stmt_irgen);

  ctx.Module().setTargetTriple(llvm::Triple("x86_64-unknown-linux-gnu"));
  for (const auto &machine : program.machines) {
    machine_irgen.PrepareMachine(machine);
  }
  for (const auto &machine : program.machines) {
    machine_irgen.EmitMachine(machine);
  }
  EmitModuleInit(ctx, program);
  EmitModuleCleanup(ctx, program);
  for (const auto &machine : program.machines) {
    EmitMachineWrappers(ctx, machine);
  }

  return IRGenResult{
      .llvm_context = ctx.TakeLLVMContext(),
      .module = ctx.TakeModule(),
      .diagnostics = ctx.TakeDiagnostics(),
  };
}

} // namespace flux
