#include "lowering_context.hpp"
#include <utility>

namespace flux {

LoweringContext::LoweringContext(const SemanticModel &semantic_model) : _semantic_model(semantic_model) {}

const SemanticModel &LoweringContext::Semantic() const { return _semantic_model; }

void LoweringContext::Error(std::string message) { _diagnostics.push_back(std::move(message)); }

const std::vector<std::string> &LoweringContext::Diagnostics() const { return _diagnostics; }

std::vector<std::string> LoweringContext::TakeDiagnostics() { return std::move(_diagnostics); }

void LoweringContext::EnterMachine(const MachineDecl &machine) {
  _current_machine = &machine;
  _scope_stack.clear();
  _state_tags.clear();
  _event_tags.clear();
  _machine_globals.clear();
  PushScope();
}

void LoweringContext::LeaveMachine() {
  _current_machine = nullptr;
  _scope_stack.clear();
  _state_tags.clear();
  _event_tags.clear();
  _machine_globals.clear();
}

const MachineDecl *LoweringContext::CurrentMachine() const { return _current_machine; }

void LoweringContext::PushScope() { _scope_stack.emplace_back(); }

void LoweringContext::PopScope() {
  if (!_scope_stack.empty()) {
    _scope_stack.pop_back();
  }
}

void LoweringContext::BindSymbol(LoweredSymbolRef symbol) {
  if (_scope_stack.empty()) {
    PushScope();
  }
  _scope_stack.back()[symbol.name] = std::move(symbol);
}

const LoweredSymbolRef *LoweringContext::LookupSymbol(const std::string &name) const {
  for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it) {
    auto symbol_it = it->find(name);
    if (symbol_it != it->end()) {
      return &symbol_it->second;
    }
  }
  return nullptr;
}

void LoweringContext::RegisterStateTag(const std::string &name, int32_t tag) { _state_tags[name] = tag; }

int32_t LoweringContext::LookupStateTag(const std::string &name) const {
  auto it = _state_tags.find(name);
  if (it == _state_tags.end()) {
    return -1;
  }
  return it->second;
}

void LoweringContext::RegisterEventTag(const std::string &name, int32_t tag) { _event_tags[name] = tag; }

int32_t LoweringContext::LookupEventTag(const std::string &name) const {
  auto it = _event_tags.find(name);
  if (it == _event_tags.end()) {
    return -1;
  }
  return it->second;
}

void LoweringContext::RegisterMachineGlobal(const std::string &name) { _machine_globals.insert(name); }

bool LoweringContext::IsKnownMachineGlobal(const std::string &name) const {
  return _machine_globals.find(name) != _machine_globals.end();
}

} // namespace flux
