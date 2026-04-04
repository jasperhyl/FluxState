#pragma once

#include "../ast/machine.hpp"
#include "../lexer/lexer.hpp"
#include "error.cpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
namespace flux {
class Parser {
public:
  explicit Parser(std::vector<Token> tokens);
  // 整个Program
  Program ParseProgram();

private:
  // for machine,名字可以直接消费掉
  MachineDecl ParseMachineDecl();
  std::vector<std::string> ParseStateDecl(MachineDecl &machine);
  std::string ParseInitialState(MachineDecl &machine);
  EventDecl ParseEventDecl(MachineDecl &machine);
  std::vector<Param> ParseParam(MachineDecl &machine);
  ValueType ParseType();
  TransitionDecl ParseTransitionDecl(MachineDecl &machine);
  // for TransitionDecl(action block  就是 do block,为了区分while do,所以叫action block)
  TransitionTrigger ParseOnEventTrigger(TransitionDecl &transition);
  TransitionTrigger ParseAfterTrigger(TransitionDecl &transition);
  std::unique_ptr<Expr> ParseGuard();
  // 只对stmt和expr采用unique_ptr统一管理，让他们在堆上分配，只保存指针，是为了简化设计
  // 而Program和MachineDecl等等依旧采用按值存
  // for DoBlock(ActionBlock)
  std ::unique_ptr<Stmt> ParseStmt();
  std::unique_ptr<BlockStmt> ParseBlockStmt();
  std::unique_ptr<ExprStmt> ParseExprStmt();
  std::unique_ptr<Expr> ParseExpr();
  std::unique_ptr<Expr> ParseAssign();
  std::unique_ptr<Expr> ParseOr();
  std::unique_ptr<Expr> ParseAnd();
  std::unique_ptr<Expr> ParseEquality();
  std::unique_ptr<Expr> ParseComparison();
  BinaryOp ParseBinaryOp(TokenType type);
  std::unique_ptr<Expr> ParseTerm();   // 加减
  std::unique_ptr<Expr> ParseFactor(); // 乘除取模
  UnaryOp ParseUnaryOp(TokenType type);
  std::unique_ptr<Expr> ParseUnary();   // 一元运算
  std::unique_ptr<Expr> ParsePrimary(); // 解析叶子层，不能再拆的小单元
  // for emit
  std::unique_ptr<Stmt> ParseEmit();
  EventInvoke ParseEventInvoke();
  // tokens
  const Token &Peek() const;
  const Token &Previous() const;
  bool IsAtEnd() const;
  bool Check(TokenType type) const;
  bool Match(TokenType type);                                   // 尝试匹配，没有也没关系
  const Token &Consume(TokenType type, const std::string &msg); // 必须匹配，没有直接报错
  std::vector<Token> _tokens;
  size_t _current = 0;
};
} // namespace flux