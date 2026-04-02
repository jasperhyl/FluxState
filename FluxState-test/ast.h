#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>

namespace llvm {
class Value;
}

// Forward declarations for code generation
llvm::Value *LogErrorV(const char *Str);

// --- Expression nodes ---
class ExprAST {
public:
  virtual ~ExprAST() = default;
  virtual llvm::Value *codegen() = 0;
  virtual void dump() const = 0;
};

class NumberExprAST : public ExprAST {
  
public:
    double Val;
  NumberExprAST(double Val) : Val(Val) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

class BoolExprAST : public ExprAST {
  bool Val;
public:
  BoolExprAST(bool Val) : Val(Val) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

class StringExprAST : public ExprAST {
  std::string Val;
public:
  StringExprAST(const std::string &Val) : Val(Val) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

class VariableExprAST : public ExprAST {
  std::string Name;
public:
  VariableExprAST(const std::string &Name) : Name(Name) {}
  llvm::Value *codegen() override;
  void dump() const override;
  const std::string &getName() const { return Name; }
};

class BinaryExprAST : public ExprAST {
  int Op;
  std::unique_ptr<ExprAST> LHS, RHS;
public:
  BinaryExprAST(int Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

class UnaryExprAST : public ExprAST {
  int Op;
  std::unique_ptr<ExprAST> Operand;
public:
  UnaryExprAST(int Op, std::unique_ptr<ExprAST> Operand)
      : Op(Op), Operand(std::move(Operand)) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;
public:
  CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args)
      : Callee(Callee), Args(std::move(Args)) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

// --- Statement nodes ---
class StmtAST {
public:
  virtual ~StmtAST() = default;
  virtual llvm::Value *codegen() = 0;
  virtual void dump() const = 0;
};

class BlockStmtAST : public StmtAST {
  std::vector<std::unique_ptr<StmtAST>> Stmts;
public:
  BlockStmtAST(std::vector<std::unique_ptr<StmtAST>> Stmts) : Stmts(std::move(Stmts)) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

class AssignStmtAST : public StmtAST {
  std::string LHS;
  std::unique_ptr<ExprAST> RHS;
public:
  AssignStmtAST(const std::string &LHS, std::unique_ptr<ExprAST> RHS)
      : LHS(LHS), RHS(std::move(RHS)) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

class EmitStmtAST : public StmtAST {
  std::string EventName;
public:
  EmitStmtAST(const std::string &EventName) : EventName(EventName) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

class IfStmtAST : public StmtAST {
  std::unique_ptr<ExprAST> Cond;
  std::unique_ptr<StmtAST> ThenStmt;
  std::unique_ptr<StmtAST> ElseStmt;
public:
  IfStmtAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StmtAST> ThenStmt,
            std::unique_ptr<StmtAST> ElseStmt = nullptr)
      : Cond(std::move(Cond)), ThenStmt(std::move(ThenStmt)), ElseStmt(std::move(ElseStmt)) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

class WhileStmtAST : public StmtAST {
  std::unique_ptr<ExprAST> Cond;
  std::unique_ptr<StmtAST> Body;
public:
  WhileStmtAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StmtAST> Body)
      : Cond(std::move(Cond)), Body(std::move(Body)) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

/// Expression statement (e.g., log("message");)
class ExpressionStmtAST : public StmtAST {
  std::unique_ptr<ExprAST> Expr;
public:
  ExpressionStmtAST(std::unique_ptr<ExprAST> Expr) : Expr(std::move(Expr)) {}
  llvm::Value *codegen() override;
  void dump() const override;
};

// --- Declaration nodes ---
class VarDeclAST {
  std::string Type;
  std::string Name;
  std::unique_ptr<ExprAST> Initializer;
public:
  VarDeclAST(const std::string &Type, const std::string &Name,
             std::unique_ptr<ExprAST> Initializer = nullptr)
      : Type(Type), Name(Name), Initializer(std::move(Initializer)) {}
  llvm::Value *codegen();
  void dump() const;
  const std::string &getName() const { return Name; }
  const std::string &getType() const { return Type; }
};

class EventDeclAST {
  std::string Name;
public:
  EventDeclAST(const std::string &Name) : Name(Name) {}
  llvm::Value *codegen();
  void dump() const;
  const std::string &getName() const { return Name; }
};

class TransitionAST {
public:
  enum class Kind { Event, After };
private:
  Kind kind;
  std::string SrcState;
  // For event kind
  std::string EventName;
  // For after kind
  double AfterSeconds;
  // Common
  std::unique_ptr<ExprAST> GuardCondition;
  std::string DstState;
  std::unique_ptr<StmtAST> Actions;
public:
  // Event constructor
  TransitionAST(const std::string &Src, const std::string &Event,
                const std::string &Dst,
                std::unique_ptr<ExprAST> Guard = nullptr,
                std::unique_ptr<StmtAST> Actions = nullptr)
      : kind(Kind::Event), SrcState(Src), EventName(Event), AfterSeconds(0.0),
        GuardCondition(std::move(Guard)), DstState(Dst), Actions(std::move(Actions)) {}
  // After constructor (now with optional Guard)
  TransitionAST(const std::string &Src, double Seconds, const std::string &Dst,
                std::unique_ptr<ExprAST> Guard = nullptr,
                std::unique_ptr<StmtAST> Actions = nullptr)
      : kind(Kind::After), SrcState(Src), AfterSeconds(Seconds),
        GuardCondition(std::move(Guard)), DstState(Dst), Actions(std::move(Actions)) {}

  llvm::Value *codegen();
  void dump() const;
  Kind getKind() const { return kind; }
  const std::string &getSrcState() const { return SrcState; }
  const std::string &getEventName() const { return EventName; }
  double getAfterSeconds() const { return AfterSeconds; }
  ExprAST* getGuardCondition() const { return GuardCondition.get(); }
  const std::string &getDstState() const { return DstState; }
  StmtAST* getActions() const { return Actions.get(); }
};

class MachineAST {
  std::string Name;
  std::vector<std::string> StateNames;
  std::string InitialState;
  std::vector<std::unique_ptr<VarDeclAST>> Variables;
  std::vector<std::unique_ptr<EventDeclAST>> Events;
  std::vector<std::unique_ptr<TransitionAST>> Transitions;
public:
  MachineAST(const std::string &Name, std::vector<std::string> StateNames,
             const std::string &InitialState,
             std::vector<std::unique_ptr<VarDeclAST>> Vars,
             std::vector<std::unique_ptr<EventDeclAST>> Events,
             std::vector<std::unique_ptr<TransitionAST>> Transitions)
      : Name(Name), StateNames(std::move(StateNames)), InitialState(InitialState),
        Variables(std::move(Vars)), Events(std::move(Events)), Transitions(std::move(Transitions)) {}

  llvm::Value *codegen();
  void dump() const;
  const std::string &getName() const { return Name; }
};

#endif // AST_H