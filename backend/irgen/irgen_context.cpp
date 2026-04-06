#include "irgen_context.hpp"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace flux {

IRGenContext::IRGenContext(const std::string &module_name)
    : _llvm_context(std::make_unique<llvm::LLVMContext>()),
      _module(std::make_unique<llvm::Module>(module_name, *_llvm_context)),
      _builder(std::make_unique<llvm::IRBuilder<>>(*_llvm_context)) {}

IRGenContext::~IRGenContext() = default;

llvm::LLVMContext &IRGenContext::LLVM() { return *_llvm_context; }

llvm::Module &IRGenContext::Module() { return *_module; }

llvm::IRBuilder<> &IRGenContext::Builder() { return *_builder; }

void IRGenContext::Error(std::string message) { _diagnostics.push_back(std::move(message)); }

const std::vector<std::string> &IRGenContext::Diagnostics() const { return _diagnostics; }

std::vector<std::string> IRGenContext::TakeDiagnostics() { return std::move(_diagnostics); }

void IRGenContext::SetRuntimeDecls(RuntimeDecls runtime_decls) { _runtime = runtime_decls; }

RuntimeDecls &IRGenContext::Runtime() { return _runtime; }

const RuntimeDecls &IRGenContext::Runtime() const { return _runtime; }

void IRGenContext::SetCurrentFunction(llvm::Function *function) { _current_function = function; }

llvm::Function *IRGenContext::CurrentFunction() const { return _current_function; }

void IRGenContext::SetCurrentMachineValue(llvm::Value *value) { _current_machine_value = value; }

llvm::Value *IRGenContext::CurrentMachineValue() const { return _current_machine_value; }

void IRGenContext::SetCurrentEventValue(llvm::Value *value) { _current_event_value = value; }

llvm::Value *IRGenContext::CurrentEventValue() const { return _current_event_value; }

void IRGenContext::SetCurrentMachineLayout(MachineIRLayout layout) { _current_machine_layout = std::move(layout); }

const MachineIRLayout *IRGenContext::CurrentMachineLayout() const {
  if (!_current_machine_layout.has_value()) {
    return nullptr;
  }
  return &*_current_machine_layout;
}

void IRGenContext::RegisterMachineInstance(std::string name, MachineInstanceIRInfo info) {
  _machine_instances[std::move(name)] = std::move(info);
}

const MachineInstanceIRInfo *IRGenContext::FindMachineInstance(const std::string &name) const {
  auto it = _machine_instances.find(name);
  if (it == _machine_instances.end()) {
    return nullptr;
  }
  return &it->second;
}

void IRGenContext::PushValueScope() { _value_scopes.emplace_back(); }

void IRGenContext::PopValueScope() {
  if (!_value_scopes.empty()) {
    _value_scopes.pop_back();
  }
}

void IRGenContext::BindValue(const std::string &name, llvm::Value *value) {
  if (_value_scopes.empty()) {
    PushValueScope();
  }
  _value_scopes.back()[name] = value;
}

llvm::Value *IRGenContext::LookupValue(const std::string &name) const {
  for (auto it = _value_scopes.rbegin(); it != _value_scopes.rend(); ++it) {
    auto value_it = it->find(name);
    if (value_it != it->end()) {
      return value_it->second;
    }
  }
  return nullptr;
}

std::unique_ptr<llvm::LLVMContext> IRGenContext::TakeLLVMContext() { return std::move(_llvm_context); }

std::unique_ptr<llvm::Module> IRGenContext::TakeModule() { return std::move(_module); }

} // namespace flux
