#include "ast.h"
#include <iostream>
#include "lexer.h"

// --------------------------------------
// Expression dump
// --------------------------------------
void NumberExprAST::dump() const {
  std::cout << Val;
}
void BoolExprAST::dump() const {
  std::cout << (Val ? "true" : "false");
}
void StringExprAST::dump() const {
  std::cout << "\"" << Val << "\"";
}
void VariableExprAST::dump() const {
  std::cout << Name;
}
void BinaryExprAST::dump() const {
  std::cout << "(";
  LHS->dump();
  std::cout << " ";
  // print operator
  switch (Op) {
    case tok_eq: std::cout << "=="; break;
    case tok_ne: std::cout << "!="; break;
    case tok_le: std::cout << "<="; break;
    case tok_ge: std::cout << ">="; break;
    case tok_lt: std::cout << "<"; break;
    case tok_gt: std::cout << ">"; break;
    case tok_and: std::cout << "&&"; break;
    case tok_or: std::cout << "||"; break;
    default: std::cout << (char)Op;
  }
  std::cout << " ";
  RHS->dump();
  std::cout << ")";
}
void UnaryExprAST::dump() const {
  std::cout << "(";
  std::cout << (char)Op;
  Operand->dump();
  std::cout << ")";
}
void CallExprAST::dump() const {
  std::cout << Callee << "(";
  for (size_t i = 0; i < Args.size(); ++i) {
    if (i > 0) std::cout << ", ";
    Args[i]->dump();
  }
  std::cout << ")";
}

// --------------------------------------
// Statement dump
// --------------------------------------
void BlockStmtAST::dump() const {
  std::cout << "{\n";
  for (const auto &stmt : Stmts) {
    stmt->dump();
    std::cout << "\n";
  }
  std::cout << "}";
}
void AssignStmtAST::dump() const {
  std::cout << LHS << " = ";
  RHS->dump();
  std::cout << ";";
}
void EmitStmtAST::dump() const {
  std::cout << "emit " << EventName << ";";
}
void IfStmtAST::dump() const {
  std::cout << "if (";
  Cond->dump();
  std::cout << ") ";
  ThenStmt->dump();
  if (ElseStmt) {
    std::cout << " else ";
    ElseStmt->dump();
  }
}
void WhileStmtAST::dump() const {
  std::cout << "while (";
  Cond->dump();
  std::cout << ") ";
  Body->dump();
}
void ExpressionStmtAST::dump() const {
  Expr->dump();
  std::cout << ";";
}

// --------------------------------------
// Declaration dump
// --------------------------------------
void VarDeclAST::dump() const {
  std::cout << Type << " " << Name;
  if (Initializer) {
    std::cout << " = ";
    Initializer->dump();
  }
  std::cout << ";";
}
void EventDeclAST::dump() const {
  std::cout << "event " << Name << "();";
}
void TransitionAST::dump() const {
  std::cout << "from " << SrcState << " ";
  if (kind == Kind::Event) {
    std::cout << "on " << EventName << "() -> " << DstState;
    if (GuardCondition) {
      std::cout << " when ";
      GuardCondition->dump();
    }
  } else {
    std::cout << "after " << AfterSeconds << "s";
    if (GuardCondition) {
      std::cout << " when ";
      GuardCondition->dump();
    }
    std::cout << " -> " << DstState;
  }
  if (Actions) {
    std::cout << " do ";
    Actions->dump();
  }
  std::cout << ";";
}
void MachineAST::dump() const {
  std::cout << "machine " << Name << " {\n";
  std::cout << "  state";
  for (const auto &s : StateNames) std::cout << " " << s;
  std::cout << ";\n";
  std::cout << "  initial " << InitialState << ";\n";
  for (const auto &var : Variables) {
    std::cout << "  ";
    var->dump();
    std::cout << "\n";
  }
  for (const auto &ev : Events) {
    std::cout << "  ";
    ev->dump();
    std::cout << "\n";
  }
  for (const auto &tr : Transitions) {
    std::cout << "  ";
    tr->dump();
    std::cout << "\n";
  }
  std::cout << "}\n";
}

// --------------------------------------
// Codegen stubs (to be implemented later)
// --------------------------------------
llvm::Value *LogErrorV(const char *Str) { return nullptr; }
llvm::Value *NumberExprAST::codegen() { return nullptr; }
llvm::Value *BoolExprAST::codegen() { return nullptr; }
llvm::Value *StringExprAST::codegen() { return nullptr; }
llvm::Value *VariableExprAST::codegen() { return nullptr; }
llvm::Value *BinaryExprAST::codegen() { return nullptr; }
llvm::Value *UnaryExprAST::codegen() { return nullptr; }
llvm::Value *CallExprAST::codegen() { return nullptr; }
llvm::Value *BlockStmtAST::codegen() { return nullptr; }
llvm::Value *AssignStmtAST::codegen() { return nullptr; }
llvm::Value *EmitStmtAST::codegen() { return nullptr; }
llvm::Value *IfStmtAST::codegen() { return nullptr; }
llvm::Value *WhileStmtAST::codegen() { return nullptr; }
llvm::Value *ExpressionStmtAST::codegen() { return Expr ? Expr->codegen() : nullptr; }
llvm::Value *VarDeclAST::codegen() { return nullptr; }
llvm::Value *EventDeclAST::codegen() { return nullptr; }
llvm::Value *TransitionAST::codegen() { return nullptr; }
llvm::Value *MachineAST::codegen() { return nullptr; }