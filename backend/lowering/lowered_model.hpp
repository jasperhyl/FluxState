#pragma once
#include "../../frontend/ast/ast.hpp"
#include "../../frontend/ast/expr.hpp"
#include "../../frontend/ast/machine.hpp"
#include "../../frontend/sema/semantic_model.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace flux {

// 后端类型系统
enum class LoweredTypeKind {
  Invalid,
  Void,
  Bool,
  Char,
  Short,
  Int,
  Float,
  Double,
  String,
  EventRef,
  MachineRef,
  OpaquePtr,
};

struct LoweredType {
  LoweredTypeKind kind = LoweredTypeKind::Invalid;
  std::string pointee_name; // 这个指针指向的对象名字,比如machine name,可以用于报错等
};

// LoweredField { name = "x", type = Int }
struct LoweredField {
  std::string name;
  // 这里有一个问题，为什么不直接存
  // LoweredTypeKind kind = LoweredTypeKind::Invalid;
  // 因为LoweredType以后还可以增加数组长度等字段
  LoweredType type;
};

struct LoweredState {
  std::string name;
  int32_t tag = -1;
};

struct LoweredEvent {
  std::string name;
  int32_t tag = -1;
  bool is_timeout = false;
  // 收集event的参数信息
  std::vector<LoweredField> params;
};

struct LoweredTimerSpec {
  int64_t delay = 0;
  DurationLiteral::Unit unit = DurationLiteral::Unit::Seconds;
  std::optional<int64_t> retry;
  int32_t timeout_event_tag = -1;
  std::string timeout_event_name;
};

// 标记不同的值来源
// 比如machine名字会被标记为MachineSymbol,表示这个名字不是普通变量，而是一个machine引用
// 比如machine 变量 lowering 时会标记成环境字段
// 因为lowering之后是用局部index标记变量的
// 如果不知道变量对应的存储类型，那么就会出现两个编号相同的不同变量
/*
 machine Counter {
  int count;

  event Add(int x);

  Idle -> Idle on Add(x) {
    count = count + x;
  }
}
lowing之后
count -> {
  name = "count",
  type = Int,
  storage = EnvField,
  index = 0
}

x -> {
  name = "x",
  type = Int,
  storage = TriggerValue,
  index = 0
}
取count:load machine.env[0]
取x:load trigger.args[0]
如果没有LoweredStorageKind，就会出现：
count -> { name = "count", type = Int, index = 0 }
x     -> { name = "x", type = Int, index = 0 }
那就不知道去哪里取了
*/
enum class LoweredStorageKind {
  None,
  MachineSymbol,
  EnvField,
  TriggerValue,
};

struct LoweredSymbolRef {
  std::string name;
  LoweredType type;
  LoweredStorageKind storage = LoweredStorageKind::None;
  size_t index = 0;
  ResolvedSymbol semantic_symbol; // 保留语义来源，但现在并没有被真正使用
};

// 标记expr节点的种类，这样才知道要怎么解释对应而expr节点
enum class LoweredExprKind {
  BoolLiteral,
  CharLiteral,
  IntLiteral,
  StringLiteral,
  Var,
  Unary,
  Binary,
  Call,
  Assign,
};

// 对于不同的LoweredExprKind，有些字段不会被设置
struct LoweredExpr {
  LoweredExprKind kind = LoweredExprKind::IntLiteral;
  LoweredType type;
  const Expr *source = nullptr; // 标记是从哪个ast节点lowering下来的

  bool bool_value = false;
  uint32_t char_value = 0;
  int32_t int_value = 0;
  std::string string_value;

  UnaryOp unary_op = UnaryOp::Plus;
  BinaryOp binary_op = BinaryOp::Add;
  LoweredSymbolRef symbol;

  std::unique_ptr<LoweredExpr> operand;
  std::unique_ptr<LoweredExpr> lhs;
  std::unique_ptr<LoweredExpr> rhs;
  std::unique_ptr<LoweredExpr> callee;
  std::vector<std::unique_ptr<LoweredExpr>> args;
};

enum class LoweredStmtKind {
  Expr,
  Block,
  Emit,
};

struct LoweredStmt {
  LoweredStmtKind kind = LoweredStmtKind::Expr;
  const Stmt *source = nullptr;

  std::unique_ptr<LoweredExpr> expr;
  std::vector<std::unique_ptr<LoweredStmt>> stmts;

  std::unique_ptr<LoweredExpr> emit_target;
  std::string emit_target_machine_name;
  int32_t emit_event_tag = -1;
  std::string emit_event_name;
  std::vector<std::unique_ptr<LoweredExpr>> emit_args;
};

enum class LoweredTriggerKind {
  OnEvent,
  After,
};

struct LoweredTransition {
  const TransitionDecl *source = nullptr;
  int32_t src_state_tag = -1;
  int32_t dst_state_tag = -1;
  LoweredTriggerKind trigger_kind = LoweredTriggerKind::OnEvent;
  int32_t trigger_id = -1;
  std::unique_ptr<LoweredExpr> guard;
  std::unique_ptr<LoweredStmt> action;
  std::optional<LoweredTimerSpec> timer;
  std::vector<LoweredSymbolRef> bindings;
};

struct LoweredEnvLayout {
  // 存machine的非业务变量的声明
  std::vector<LoweredField> fields;
  // 每个字段对应的初始化表达式（没有初始化，这里就会为空）
  std::vector<std::unique_ptr<LoweredExpr>> initializers;
};

struct LoweredMachine {
  const MachineDecl *source = nullptr;
  std::string name;
  int32_t initial_state_tag = -1;
  LoweredEnvLayout env;
  std::vector<LoweredState> states;
  std::vector<LoweredEvent> events;
  std::vector<LoweredTransition> transitions;
};

struct LoweredProgram {
  std::vector<LoweredMachine> machines;
};

inline LoweredType LoweredTypeFromValueType(ValueType type) {
  switch (type) {
  case ValueType::Bool:
    return LoweredType{.kind = LoweredTypeKind::Bool};
  case ValueType::MachineRef:
    return LoweredType{.kind = LoweredTypeKind::MachineRef};
  case ValueType::Char:
    return LoweredType{.kind = LoweredTypeKind::Char};
  case ValueType::Short:
    return LoweredType{.kind = LoweredTypeKind::Short};
  case ValueType::Int:
    return LoweredType{.kind = LoweredTypeKind::Int};
  case ValueType::Float:
    return LoweredType{.kind = LoweredTypeKind::Float};
  case ValueType::Double:
    return LoweredType{.kind = LoweredTypeKind::Double};
  case ValueType::String:
    return LoweredType{.kind = LoweredTypeKind::String};
  case ValueType::Invalid:
  default:
    return LoweredType{.kind = LoweredTypeKind::Invalid};
  }
}

} // namespace flux
