#pragma once
#include "../ast/machine.hpp"
#include "../ast/stmt.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace flux {

enum class SymbolKind {
  MachineName,
  MachineVar,
  TriggerBinding,
};

struct ResolvedSymbol {
  SymbolKind kind = SymbolKind::MachineVar;
  std::string name;
  ValueType type = ValueType::Invalid;
  const MachineDecl *machine_decl = nullptr;
  const VarDecl *var_decl = nullptr;
  const Param *param = nullptr;
};

struct MachineSemanticInfo {
  const MachineDecl *decl = nullptr;
  std::unordered_set<std::string> states;
  std::unordered_map<std::string, const EventDecl *> events;
};

class SemanticModel {
public:
  void RecordExprType(const Expr &expr, ValueType type) { _expr_types[&expr] = type; }

  std::optional<ValueType> FindExprType(const Expr &expr) const {
    auto it = _expr_types.find(&expr);
    if (it == _expr_types.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  void RecordVarBinding(const VarExpr &expr, ResolvedSymbol symbol) { _var_bindings[&expr] = std::move(symbol); }

  const ResolvedSymbol *FindVarBinding(const VarExpr &expr) const {
    auto it = _var_bindings.find(&expr);
    if (it == _var_bindings.end()) {
      return nullptr;
    }
    return &it->second;
  }

  void RecordOnEventResolution(const OnEventTrigger &trigger, const EventDecl &event) { _on_event_bindings[&trigger] = &event; }

  const EventDecl *FindOnEventResolution(const OnEventTrigger &trigger) const {
    auto it = _on_event_bindings.find(&trigger);
    if (it == _on_event_bindings.end()) {
      return nullptr;
    }
    return it->second;
  }

  void RecordEmitResolution(const EmitStmt &stmt, const MachineDecl &target_machine, const EventDecl &event) {
    _emit_bindings[&stmt] = EmitResolution{
        .target_machine = &target_machine,
        .event = &event,
    };
  }

  const EventDecl *FindEmitResolution(const EmitStmt &stmt) const {
    auto it = _emit_bindings.find(&stmt);
    if (it == _emit_bindings.end()) {
      return nullptr;
    }
    return it->second.event;
  }

  const MachineDecl *FindEmitTargetMachine(const EmitStmt &stmt) const {
    auto it = _emit_bindings.find(&stmt);
    if (it == _emit_bindings.end()) {
      return nullptr;
    }
    return it->second.target_machine;
  }

  void RecordMachineInfo(const std::string &name, MachineSemanticInfo info) { _machine_infos[name] = std::move(info); }

  const MachineSemanticInfo *FindMachineInfo(const std::string &name) const {
    auto it = _machine_infos.find(name);
    if (it == _machine_infos.end()) {
      return nullptr;
    }
    return &it->second;
  }

  const std::unordered_map<const Expr *, ValueType> &ExprTypes() const { return _expr_types; }
  const std::unordered_map<const VarExpr *, ResolvedSymbol> &VarBindings() const { return _var_bindings; }
  const std::unordered_map<std::string, MachineSemanticInfo> &MachineInfos() const { return _machine_infos; }

private:
  struct EmitResolution {
    const MachineDecl *target_machine = nullptr;
    const EventDecl *event = nullptr;
  };

  std::unordered_map<const Expr *, ValueType> _expr_types;
  std::unordered_map<const VarExpr *, ResolvedSymbol> _var_bindings;
  std::unordered_map<const OnEventTrigger *, const EventDecl *> _on_event_bindings;
  std::unordered_map<const EmitStmt *, EmitResolution> _emit_bindings;
  std::unordered_map<std::string, MachineSemanticInfo> _machine_infos;
};

} // namespace flux
