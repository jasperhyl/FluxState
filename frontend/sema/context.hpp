#pragma once
#include "../ast/machine.hpp"
#include "diagnostics.hpp"
#include "semantic_model.hpp"
#include "scope.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>

/*
保存：
所有 machine 的语义信息
当前正在分析哪个 machine
当前的作用域栈
所有诊断信息
*/

namespace flux {
// 因为AST已经存储了需要的信息，所以Info结构体只是用来存储sema阶段需要快速检索的信息
// 同时为以后的可能的扩展留下空间，而不是完全依赖于AST

struct EventInfo {
  const EventDecl *decl = nullptr;
};

// 为每个machine建立状态表和事件表
struct MachineInfo {
  const MachineDecl *decl = nullptr;
  std::unordered_set<std::string> states;
  std::unordered_map<std::string, EventInfo> events;
};

class Context {
public:
  // 诊断信息，支持ctx.Error("msg")调用
  void Report(Diagnostic::Level level, std::string msg);
  void Error(std::string msg);
  void Warning(std::string msg);

  const std::vector<Diagnostic> &Diagnostics() const;
  std::vector<Diagnostic> TakeDiagnostics();

  // machine info
  bool DeclareMachine(const MachineDecl &machine);

  MachineInfo *FindMachine(const std::string &name);
  const MachineInfo *FindMachine(const std::string &name) const;
  const std::unordered_map<std::string, MachineInfo> &Machines() const;

  EventInfo *FindEvent(const std::string &machine_name, const std::string &event_name);
  const EventInfo *FindEvent(const std::string &machine_name, const std::string &event_name) const;

  bool HasState(const std::string &machine_name, const std::string &state_name) const;

  // current machine
  void EnterMachine(const MachineDecl &machine);
  void LeaveMachine();

  const MachineDecl *CurrentMachineDecl() const;
  MachineInfo *CurrentMachineInfo();
  const MachineInfo *CurrentMachineInfo() const;

  // scope
  void PushScope();
  void PopScope();
  Scope *CurrentScope();
  const Scope *CurrentScope() const;

  bool DeclareSymbol(ResolvedSymbol symbol);
  bool DeclareMachineNameSymbol(const MachineDecl &machine);
  bool DeclareMachineVarSymbol(const VarDecl &decl);
  bool DeclareTriggerBindingSymbol(const Param &param, const std::string &binding_name);
  std::optional<ValueType> LookupSymbol(const std::string &name) const;
  std::optional<ResolvedSymbol> ResolveSymbol(const std::string &name) const;

private:
  void DeclareBuiltInMachineSymbols();
  bool CollectStates(const MachineDecl &machine, MachineInfo &info);
  bool CollectEvents(const MachineDecl &machine, MachineInfo &info);

  std::vector<Diagnostic> _diagnostics;
  std::unordered_map<std::string, MachineInfo> _machines;

  const MachineDecl *_current_machine = nullptr;
  std::vector<std::unique_ptr<Scope>> _scope_stack;
};

//

} // namespace flux