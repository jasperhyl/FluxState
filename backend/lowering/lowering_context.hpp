#pragma once
#include "../../frontend/sema/semantic_model.hpp"
#include "lowered_model.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace flux {

class LoweringContext {
public:
  // 构造函数，接受semantic model，并设置私有成员字段
  explicit LoweringContext(const SemanticModel &semantic_model);
  // 返回当前的Semantic model
  const SemanticModel &Semantic() const;
  // 向diagnostics push back message
  void Error(std::string message);
  // 返回_diagnostics
  const std::vector<std::string> &Diagnostics() const;
  // 返回std::move(_diagnostics)，lowering阶段结束后
  // 会把所有的诊断信息交给LoweringResult
  std::vector<std::string> TakeDiagnostics();
  // 进入一个machine要切换上下文，即设置下方的私有成员变量
  // 不会清空诊断信息
  void EnterMachine(const MachineDecl &machine);
  void LeaveMachine();
  const MachineDecl *CurrentMachine() const;

  void PushScope();
  void PopScope();
  // 把一个变量名绑定到对应的lowered 符号
  void BindSymbol(LoweredSymbolRef symbol);
  // 按作用域从里到外查找，然后返回ref,一个ref除了包括名字和类型，还包括他存在哪里，在那个存储里的编号，和对应的前端信息等
  const LoweredSymbolRef *LookupSymbol(const std::string &name) const;

  // 往map里存state和event的编号
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
  // 映射变量名和对应的lowered的符号的结构体LoweredSymbolRef
  std::vector<std::unordered_map<std::string, LoweredSymbolRef>> _scope_stack;
  std::unordered_map<std::string, int32_t> _state_tags; // 存state的名和编号映射
  std::unordered_map<std::string, int32_t> _event_tags; // 存event的名和编号映射
  std::unordered_set<std::string> _machine_globals;     // 把全局的machine名字都存在一起
};

} // namespace flux
