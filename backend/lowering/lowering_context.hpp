#pragma once
#include "lowered_model.hpp"
#include "../../frontend/sema/semantic_model.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace flux {

class LoweringContext {
public:
  explicit LoweringContext(const SemanticModel &semantic_model);

  const SemanticModel &Semantic() const;

  void Error(std::string message);
  const std::vector<std::string> &Diagnostics() const;
  std::vector<std::string> TakeDiagnostics();

  void EnterMachine(const MachineDecl &machine);
  void LeaveMachine();
  const MachineDecl *CurrentMachine() const;

  void PushScope();
  void PopScope();
  void BindSymbol(LoweredSymbolRef symbol);
  const LoweredSymbolRef *LookupSymbol(const std::string &name) const;

  void RegisterStateTag(const std::string &name, int32_t tag);
  int32_t LookupStateTag(const std::string &name) const;

  void RegisterEventTag(const std::string &name, int32_t tag);
  int32_t LookupEventTag(const std::string &name) const;

  void RegisterMachineGlobal(const std::string &name);
  bool IsKnownMachineGlobal(const std::string &name) const;

private:
  const SemanticModel &_semantic_model;
  std::vector<std::string> _diagnostics;
  const MachineDecl *_current_machine = nullptr;
  std::vector<std::unordered_map<std::string, LoweredSymbolRef>> _scope_stack;
  std::unordered_map<std::string, int32_t> _state_tags;
  std::unordered_map<std::string, int32_t> _event_tags;
  std::unordered_set<std::string> _machine_globals;
};

} // namespace flux
