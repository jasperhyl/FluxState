#pragma once
#include "../lowering/lowered_model.hpp"
#include <llvm/IR/IRBuilder.h>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
class Function;
class Value;
class StructType;
class GlobalVariable;
}

namespace flux {

struct RuntimeDecls {
  llvm::Function *enqueue_event = nullptr;
  llvm::Function *start_timer = nullptr;
  llvm::Function *cancel_timer = nullptr;
  llvm::Function *init_machine = nullptr;
  llvm::Function *cleanup_machine = nullptr;
  llvm::Function *runtime_step = nullptr;
  llvm::Function *runtime_run = nullptr;
  llvm::Function *stop_machine = nullptr;
  llvm::Function *malloc_fn = nullptr;
};

struct MachineIRLayout {
  const LoweredMachine *machine = nullptr;
  llvm::StructType *header_type = nullptr;
  llvm::StructType *env_type = nullptr;
  llvm::StructType *machine_type = nullptr;
  std::unordered_map<std::string, size_t> env_field_indices;
  std::unordered_map<int32_t, llvm::StructType *> payload_types_by_event_tag;
  std::unordered_map<int32_t, const LoweredEvent *> events_by_tag;
};

struct MachineInstanceIRInfo {
  MachineIRLayout layout;
  llvm::GlobalVariable *global = nullptr;
};

class IRGenContext {
public:
  explicit IRGenContext(const std::string &module_name);
  ~IRGenContext();

  llvm::LLVMContext &LLVM();
  llvm::Module &Module();
  llvm::IRBuilder<> &Builder();

  void Error(std::string message);
  const std::vector<std::string> &Diagnostics() const;
  std::vector<std::string> TakeDiagnostics();

  void SetRuntimeDecls(RuntimeDecls runtime_decls);
  RuntimeDecls &Runtime();
  const RuntimeDecls &Runtime() const;

  void SetCurrentFunction(llvm::Function *function);
  llvm::Function *CurrentFunction() const;

  void SetCurrentMachineValue(llvm::Value *value);
  llvm::Value *CurrentMachineValue() const;

  void SetCurrentEventValue(llvm::Value *value);
  llvm::Value *CurrentEventValue() const;

  void SetCurrentMachineLayout(MachineIRLayout layout);
  const MachineIRLayout *CurrentMachineLayout() const;

  void RegisterMachineInstance(std::string name, MachineInstanceIRInfo info);
  const MachineInstanceIRInfo *FindMachineInstance(const std::string &name) const;

  void PushValueScope();
  void PopValueScope();
  void BindValue(const std::string &name, llvm::Value *value);
  llvm::Value *LookupValue(const std::string &name) const;

  std::unique_ptr<llvm::LLVMContext> TakeLLVMContext();
  std::unique_ptr<llvm::Module> TakeModule();

private:
  std::unique_ptr<llvm::LLVMContext> _llvm_context;
  std::unique_ptr<llvm::Module> _module;
  std::unique_ptr<llvm::IRBuilder<>> _builder;
  std::vector<std::string> _diagnostics;
  RuntimeDecls _runtime;
  llvm::Function *_current_function = nullptr;
  llvm::Value *_current_machine_value = nullptr;
  llvm::Value *_current_event_value = nullptr;
  std::optional<MachineIRLayout> _current_machine_layout;
  std::unordered_map<std::string, MachineInstanceIRInfo> _machine_instances;
  std::vector<std::unordered_map<std::string, llvm::Value *>> _value_scopes;
};

} // namespace flux
