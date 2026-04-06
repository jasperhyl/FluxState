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
  std::string pointee_name;
};

struct LoweredField {
  std::string name;
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
  std::vector<LoweredField> params;
};

struct LoweredTimerSpec {
  int64_t delay = 0;
  DurationLiteral::Unit unit = DurationLiteral::Unit::Seconds;
  std::optional<int64_t> retry;
  int32_t timeout_event_tag = -1;
  std::string timeout_event_name;
};

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
  ResolvedSymbol semantic_symbol;
};

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

struct LoweredExpr {
  LoweredExprKind kind = LoweredExprKind::IntLiteral;
  LoweredType type;
  const Expr *source = nullptr;

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
  std::vector<LoweredField> fields;
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
