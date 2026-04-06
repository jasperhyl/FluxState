#include "context.hpp"
#include "diagnostics.hpp"
#include "scope.hpp"
#include <memory>
#include <unordered_set>
#include <vector>
namespace flux {
void Context::Report(Diagnostic::Level level, std::string msg) {
  _diagnostics.push_back(Diagnostic{.level = level, .msg = std::move(msg)});
}

void Context::Error(std::string msg) {
  _diagnostics.push_back(Diagnostic{.level = Diagnostic::Level::Error, .msg = std::move(msg)});
}
void Context::Warning(std::string msg) {
  _diagnostics.push_back(Diagnostic{.level = Diagnostic::Level::Warning, .msg = std::move(msg)});
}

const std::vector<Diagnostic> &Context::Diagnostics() const { return _diagnostics; }

std::vector<Diagnostic> Context::TakeDiagnostics() { return std::move(_diagnostics); }

bool Context::DeclareMachine(const MachineDecl &machine) {
  if (_machines.find(machine.name) != _machines.end()) {
    Error("Duplicate machine '" + machine.name + "'");
    return false;
  }
  MachineInfo info;
  info.decl = &machine;
  CollectStates(machine, info);
  CollectEvents(machine, info);

  _machines.emplace(machine.name, std::move(info));
  return true;
}

bool Context::CollectStates(const MachineDecl &machine, MachineInfo &info) {
  bool flag = true;
  for (const auto &state : machine.states) {
    if (!info.states.emplace(state).second) {
      Error("duplicate state '" + state + "' in machine '" + machine.name + "'");
      flag = false;
    }
  }
  return flag;
}

bool Context::CollectEvents(const MachineDecl &machine, MachineInfo &info) {
  bool flag = true;
  for (const auto &event : machine.events) {
    if (info.events.find(event.name) != info.events.end()) {
      flag = false;
      Error("duplicate event '" + event.name + "' in machine '" + machine.name + "'");
      continue;
    }
    // 检查参数名是否重复
    std::unordered_set<std::string> param_names;
    for (const auto &param : event.params) {
      if (!param_names.emplace(param.name).second) {
        Error("duplicate parameter '" + param.name + "' in event '" + event.name + "' of machine '" + machine.name +
              "'");
        flag = false;
      }
    }
    info.events.emplace(event.name, EventInfo{.decl = &event});
  }
  return flag;
}

MachineInfo *Context::FindMachine(const std::string &name) {
  auto it = _machines.find(name);
  if (it == _machines.end()) {
    return nullptr;
  }
  return &it->second;
}

const MachineInfo *Context::FindMachine(const std::string &name) const {
  auto it = _machines.find(name);
  if (it == _machines.end()) {
    return nullptr;
  }
  return &it->second;
}

const std::unordered_map<std::string, MachineInfo> &Context::Machines() const { return _machines; }

EventInfo *Context::FindEvent(const std::string &machine_name, const std::string &event_name) {
  MachineInfo *machine = FindMachine(machine_name);
  if (machine == nullptr) {
    return nullptr;
  }

  auto it = machine->events.find(event_name);
  if (it == machine->events.end()) {
    return nullptr;
  }
  return &it->second;
}

const EventInfo *Context::FindEvent(const std::string &machine_name, const std::string &event_name) const {
  const MachineInfo *machine = FindMachine(machine_name);
  if (machine == nullptr) {
    return nullptr;
  }

  auto it = machine->events.find(event_name);
  if (it == machine->events.end()) {
    return nullptr;
  }
  return &it->second;
}

bool Context::HasState(const std::string &machine_name, const std::string &state_name) const {
  const MachineInfo *machine = FindMachine(machine_name);
  if (machine == nullptr) {
    return false;
  }
  return machine->states.find(state_name) != machine->states.end();
}

void Context::EnterMachine(const MachineDecl &machine) {
  _current_machine = &machine;
  _scope_stack.clear();
  PushScope();
  DeclareBuiltInMachineSymbols();
}

void Context::LeaveMachine() {
  _current_machine = nullptr;
  _scope_stack.clear();
}

const MachineDecl *Context::CurrentMachineDecl() const { return _current_machine; }

MachineInfo *Context::CurrentMachineInfo() {
  if (_current_machine == nullptr) {
    return nullptr;
  }
  return FindMachine(_current_machine->name);
}

const MachineInfo *Context::CurrentMachineInfo() const {
  if (_current_machine == nullptr) {
    return nullptr;
  }
  return FindMachine(_current_machine->name);
}

void Context::PushScope() {
  const Scope *parent = _scope_stack.empty() ? nullptr : _scope_stack.back().get();
  _scope_stack.push_back(std::make_unique<Scope>(parent)); // 创建新作用域，并把父作用域传入
}

void Context::PopScope() {
  if (!_scope_stack.empty()) {
    _scope_stack.pop_back();
  }
}

Scope *Context::CurrentScope() {
  if (_scope_stack.empty()) {
    return nullptr;
  }
  return _scope_stack.back().get();
}

const Scope *Context::CurrentScope() const {
  if (_scope_stack.empty()) {
    return nullptr;
  }
  return _scope_stack.back().get();
}

bool Context::DeclareSymbol(ResolvedSymbol symbol) {
  if (_scope_stack.empty()) {
    PushScope();
  }

  Scope *scope = _scope_stack.back().get();
  if (!scope->Declare(symbol)) {
    Error("duplicate symbol '" + symbol.name + "' in current scope");
    return false;
  }
  return true;
}

bool Context::DeclareMachineNameSymbol(const MachineDecl &machine) {
  return DeclareSymbol(ResolvedSymbol{
      .kind = SymbolKind::MachineName,
      .name = machine.name,
      .type = ValueType::MachineRef,
      .machine_decl = &machine,
      .var_decl = nullptr,
      .param = nullptr,
  });
}

bool Context::DeclareMachineVarSymbol(const VarDecl &decl) {
  return DeclareSymbol(ResolvedSymbol{
      .kind = SymbolKind::MachineVar,
      .name = decl.name,
      .type = decl.type,
      .machine_decl = nullptr,
      .var_decl = &decl,
      .param = nullptr,
  });
}

bool Context::DeclareTriggerBindingSymbol(const Param &param, const std::string &binding_name) {
  return DeclareSymbol(ResolvedSymbol{
      .kind = SymbolKind::TriggerBinding,
      .name = binding_name,
      .type = param.type,
      .machine_decl = nullptr,
      .var_decl = nullptr,
      .param = &param,
  });
}

void Context::DeclareBuiltInMachineSymbols() {
  for (const auto &[name, info] : _machines) {
    if (info.decl != nullptr) {
      DeclareMachineNameSymbol(*info.decl);
    }
  }
}

std::optional<ValueType> Context::LookupSymbol(const std::string &name) const {
  auto symbol = ResolveSymbol(name);
  if (!symbol.has_value()) {
    return std::nullopt;
  }
  return symbol->type;
}

std::optional<ResolvedSymbol> Context::ResolveSymbol(const std::string &name) const {
  if (_scope_stack.empty()) {
    return std::nullopt;
  }
  return _scope_stack.back()->Lookup(name);
}

} // namespace flux