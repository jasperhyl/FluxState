#pragma once
#include "parser.hpp"
#include "../ast/expr.hpp"
#include "../ast/machine.hpp"
#include "error.cpp"
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
namespace flux {
std::unique_ptr<Expr> MakeBinary(std::unique_ptr<Expr> lhs, BinaryOp op, std::unique_ptr<Expr> rhs) {
  auto n = std::make_unique<BinaryExpr>();
  n->lhs = std::move(lhs);
  n->op = std::move(op);
  n->rhs = std::move(rhs);
  return n;
}

Parser::Parser(std::vector<Token> tokens) : _tokens(std::move(tokens)) {}

Program Parser::ParseProgram() {
  Program program;
  while (!IsAtEnd()) {
    program.machines.push_back(ParseMachineDecl());
  }
  return program;
}

const Token &Parser::Peek() const { return _tokens[_current]; }

const Token &Parser::Previous() const { return _tokens[_current - 1]; }

bool Parser::IsAtEnd() const { return Peek().type == TokenType::EndOfFile; }

bool Parser::Check(TokenType type) const { return Peek().type == type; }

bool Parser::Match(TokenType type) {
  if (!Check(type))
    return false;
  _current++;
  return true;
}
const Token &Parser::Consume(TokenType type, const std::string &msg) {
  if (!Check(type)) {
    throw std::runtime_error(BuildParseError(Peek(), msg));
  }
  _current++;
  return Previous(); // consume会消费字符，但是我们依旧需要前一个字符用于信息保存，比如token.line还有token.lexeme
}

MachineDecl Parser::ParseMachineDecl() {
  MachineDecl machine;
  Consume(TokenType::KwMachine, "expect 'machine' ");
  machine.name = Consume(TokenType::Identifier, "expect machine name").lexeme;
  Consume(TokenType::LBrace, "expect '{' after machine name");
  while (!Check(TokenType::RBrace) && !IsAtEnd()) {
    if (Check(TokenType::KwState)) {
      auto states = ParseStateDecl(machine);
      machine.states.insert(machine.states.end(), states.begin(), states.end());
    } else if (Check(TokenType::KwInitial)) {
      machine.initial_state = ParseInitialState(machine);
    } else if (Check(TokenType::KwEvent)) {
      auto event = ParseEventDecl(machine);
      machine.events.push_back(event);
    } else if (Check(TokenType::KwFrom)) {
      auto transition = ParseTransitionDecl(machine);
      machine.transitions.push_back(transition);
    } else {
      throw std::runtime_error(BuildParseError(Peek(), "find unexpect token in machine body"));
    }
  }

  Consume(TokenType::LBrace, "expect '}' ");
  return machine;
}

std::vector<std::string> Parser::ParseStateDecl(MachineDecl &machine) {
  Consume(TokenType::KwState, "expect keyword state");
  std::vector<std::string> states;
  do {
    states.push_back(Consume(TokenType::Identifier, "expect state name").lexeme);
  } while (Match(TokenType::Comma));
  return states;
}

std::string Parser::ParseInitialState(MachineDecl &machine) {
  Consume(TokenType::KwInitial, "expect keyword initial");
  std::string state;
  if (Check(TokenType::Identifier)) {
    state = Consume(TokenType::Identifier, "expect initial state").lexeme;
  } else {
    throw std::runtime_error(BuildParseError(Peek(), "expect initial state"));
  }
  return state;
}

EventDecl Parser::ParseEventDecl(MachineDecl &machine) {
  EventDecl event;
  std::vector<Param> params;
  Consume(TokenType::KwEvent, "expect keyword event");
  while (!Check(TokenType::Semicolon) && !IsAtEnd()) {
    event.name = Consume(TokenType::Identifier, "expect event name").lexeme;
    Consume(TokenType::LParen, "expect '(' ");
    params = ParseParam(machine);
    event.params.insert(event.params.end(), params.begin(), params.end());
    Consume(TokenType::RParen, "expect ')'");
  }
  return event;
}

ValueType Parser::ParseType() {
  if (Match(TokenType::KwInt))
    return ValueType::Int;
  if (Match(TokenType::KwShort))
    return ValueType::Short;
  if (Match(TokenType::KwFloat))
    return ValueType::Float;
  if (Match(TokenType::KwDouble))
    return ValueType::Double;
  if (Match(TokenType::KwChar))
    return ValueType::Char;
  if (Match(TokenType::KwBool))
    return ValueType::Bool;
  if (Match(TokenType::KwString))
    return ValueType::String;

  throw std::runtime_error(BuildParseError(Peek(), "expect parameter type"));
}

std::vector<Param> Parser::ParseParam(MachineDecl &machine) {
  std::vector<Param> params;
  do {
    Param param;
    param.type = ParseType();
    param.name = Consume(TokenType::Identifier, "expect param name").lexeme;
    params.push_back(std::move(param));
  } while (Match(TokenType::Comma));
  return params;
}

TransitionDecl Parser::ParseTransitionDecl(MachineDecl &machine) {
  Consume(TokenType::KwFrom, "expect keyword from");
  TransitionDecl transition;
  transition.src_state = Consume(TokenType::Identifier, "expect src state").lexeme;
  if (Check(TokenType::KwOn)) {
    transition.trigger = ParseOnEventTrigger(transition);
  } else if (Check(TokenType::KwAfter)) {
    transition.trigger = ParseAfterTrigger(transition);
  } else {
    throw std::runtime_error(BuildParseError(Peek(), "expect keyword on or after"));
  }
  if (Check(TokenType::KwWhen)) {
    transition.guard = std::move(ParseGuard());
  }
  Consume(TokenType::Arrow, "expect arrow");
  transition.dst_state = Consume(TokenType::Identifier, "expect dst state").lexeme;
  if (Check(TokenType::KwDo)) {
    auto actions = ParseBlockStmt();
    transition.actions = std::move(actions);
  }
  return transition;
}

std::unique_ptr<Expr> Parser::ParseGuard() {
  Consume(TokenType::KwWhen, "expect keyword when");
  auto expr = ParseExpr();
  if (!expr) {
    throw std::runtime_error(BuildParseError(Peek(), "expect guard expr"));
  }
  return expr;
}

/*
Expr       -> Assign
Assign     -> Or ('=' Assign)?
Or         -> And ('||' And)*
And        -> Equality ('&&' Equality)*
Equality   -> Comparison (('==' | '!=') Comparison)*
Comparison -> Term (('>' | '>=' | '<' | '<=') Term)*
Term       -> Factor (('+' | '-') Factor)*
Factor     -> Unary (('*' | '/') Unary)*
Unary      -> ('!' | '-' | '++' | '--') Unary | Primary
Primary    -> literal | identifier | '(' Expr ')'
*/

std::unique_ptr<Expr> Parser::ParseExpr() { return ParseAssign(); }

std::unique_ptr<Expr> Parser::ParseAssign() {
  auto lhs = ParseOr();           // 解析高于等于OR的表达式
  if (Match(TokenType::Assign)) { // 支持链式赋值
    auto rhs = ParseAssign();
    auto node = std::make_unique<AssignExpr>();
    node->lhs = std::move(lhs);
    node->rhs = std::move(rhs);

    lhs = std::move(node);
  }
  return lhs;
}

// Or         -> And ('||' And)*
std::unique_ptr<Expr> Parser::ParseOr() {
  auto lhs = ParseAnd();
  while (Match(TokenType::OrOr)) {
    // 左结合
    auto rhs = ParseAnd();
    auto expr = std::make_unique<BinaryExpr>();
    expr->lhs = std::move(lhs);
    expr->op = BinaryOp::LogicOr;
    expr->rhs = std::move(rhs);

    lhs = std::move(expr);
  }
  return lhs;
}

BinaryOp Parser::ParseBinaryOp(TokenType type) {
  if (type == TokenType::AndAnd)
    return BinaryOp::LogicAnd;
  if (type == TokenType::OrOr)
    return BinaryOp::LogicOr;
  if (type == TokenType::Plus)
    return BinaryOp::Add;
  if (type == TokenType::Minus)
    return BinaryOp::Sub;
  if (type == TokenType::Star)
    return BinaryOp::Mul;
  if (type == TokenType::Slash)
    return BinaryOp::Div;
  if (type == TokenType::EqualEqual)
    return BinaryOp::Eq;
  if (type == TokenType::BangEqual)
    return BinaryOp::Ne;
  if (type == TokenType::Less)
    return BinaryOp::Lt;
  if (type == TokenType::LessEqual)
    return BinaryOp::Le;
  if (type == TokenType::Greater)
    return BinaryOp::Gt;
  if (type == TokenType::GreaterEqual)
    return BinaryOp::Ge;

  throw std::runtime_error(BuildParseError(Peek(), "expect binary op"));
}

// And        -> Equality ('&&' Equality)*
std::unique_ptr<Expr> Parser::ParseAnd() {
  auto lhs = ParseEquality();
  while (Match(TokenType::AndAnd)) {
    auto rhs = ParseEquality();
    auto expr = std::make_unique<BinaryExpr>();
    expr->lhs = std::move(lhs);
    expr->op = BinaryOp::LogicAnd;
    expr->rhs = std::move(rhs);

    lhs = std::move(expr);
  }
  return lhs;
}

// Equality   -> Comparison (('==' | '!=') Comparison)*
std::unique_ptr<Expr> Parser::ParseEquality() {
  auto lhs = ParseComparison();
  while (Match(TokenType::EqualEqual) || Match(TokenType::BangEqual)) {
    Token token = Previous();
    auto rhs = ParseComparison();
    auto expr = std::make_unique<BinaryExpr>();
    expr->lhs = std::move(lhs);
    expr->op = ParseBinaryOp(token.type);
    expr->rhs = std::move(rhs);

    lhs = std::move(expr);
  }
  return lhs;
}

// Comparison -> Term (('>' | '>=' | '<' | '<=') Term)*
std::unique_ptr<Expr> Parser::ParseComparison() {
  auto lhs = ParseTerm();
  while (Match(TokenType::Less) || Match(TokenType::LessEqual) || Match(TokenType::Greater) ||
         Match(TokenType::GreaterEqual)) {
    Token token = Previous();
    auto rhs = ParseTerm();
    auto expr = std::make_unique<BinaryExpr>();
    expr->lhs = std::move(lhs);
    expr->op = ParseBinaryOp(token.type);
    expr->rhs = std::move(rhs);

    lhs = std::move(expr);
  }
  return lhs;
}

// Term       -> Factor (('+' | '-') Factor)*
std::unique_ptr<Expr> Parser::ParseTerm() {
  auto lhs = ParseFactor();
  while (Match(TokenType::Plus) || Match(TokenType::Minus)) {
    Token token = Previous();
    auto rhs = ParseFactor();
    auto expr = std::make_unique<BinaryExpr>();
    expr->lhs = std::move(lhs);
    expr->op = ParseBinaryOp(token.type);
    expr->rhs = std::move(rhs);

    lhs = std::move(expr);
  }
  return lhs;
}

// Factor     -> Unary (('*' | '/') Unary)*
std::unique_ptr<Expr> Parser::ParseFactor() {
  auto lhs = ParseUnary();
  while (Match(TokenType::Star) || Match(TokenType::Slash)) {
    auto op = Previous();
    auto rhs = ParseUnary();
    auto expr = std::make_unique<BinaryExpr>();
    expr->lhs = std::move(lhs);
    expr->op = ParseBinaryOp(op.type);
    expr->rhs = std::move(rhs);

    lhs = std::move(expr);
  }
  return lhs;
}

UnaryOp ParseUnaryOp(TokenType type) {
  if (type == TokenType::Minus)
    return UnaryOp::Neg;
  if (type == TokenType::Plus)
    return UnaryOp::Plus;
  if (type == TokenType::Star)
    return UnaryOp::Deref;
  if (type == TokenType::Ampersand)
    return UnaryOp::AddressOf;
  // TODO:不知道要不要增加++和--，暂时不加
}

// Unary      -> ('-' | '++' | '--') Unary | Primary
std::unique_ptr<Expr> Parser::ParseUnary() {
  if (Match(TokenType::Minus) || Match(TokenType::Increment) || Match(TokenType::Decrement)) {
    Token token = Previous();
    auto operand = ParseUnary();
    auto expr = std::make_unique<UnaryExpr>();
    expr->op = ParseUnaryOp(token.type);
    expr->operand = std::move(operand);
    return expr;
  }
  return ParsePrimary();
}

// Primary    -> literal | identifier | '(' Expr ')'
std::unique_ptr<Expr> Parser::ParsePrimary() {
  if (Match(TokenType::IntegerLiteral)) {
    auto expr = std::make_unique<IntLiteralExpr>();
    expr->value = std::stoi(Previous().lexeme);
    return expr;
  }
  if (Match(TokenType::StringLiteral)) {
    auto expr = std::make_unique<StringLiteralExpr>();
    expr->value = std::move(Previous().lexeme);
    return expr;
  }
  if (Match(TokenType::Identifier)) {
    auto expr = std::make_unique<VarExpr>();
    expr->name = std::move(Previous().lexeme);
    return expr;
  }
  if (Match(TokenType::LParen)) {
    auto expr = ParseExpr();
    Consume(TokenType::RParen, "expect ')'");
    return expr;
  }
  throw std::runtime_error(BuildParseError(Peek(), "expect primary"));
}

// std::unique_ptr<Expr> Parser::ParseExpr() { return ParseAssign(); }

std ::unique_ptr<Stmt> Parser::ParseStmt() {
  // TODO:补全if和while语句
  if (Check(TokenType::LBrace)) {
    return ParseBlockStmt();
  }
  if (Check(TokenType::KwEmit)) {
    return ParseEmit();
  }
  return ParseExprStmt();
}
std::unique_ptr<ExprStmt> Parser::ParseExprStmt() {
  auto expr = ParseExpr();
  Consume(TokenType::Semicolon, "expect ';' ");
  auto stmt = std::make_unique<ExprStmt>();
  stmt->expr = std::move(expr);
  return stmt;
}

std::unique_ptr<BlockStmt> Parser::ParseBlockStmt() {
  Consume(TokenType::LBrace, "expect '{' ");
  auto block = std::make_unique<BlockStmt>();
  while (!Check(TokenType::RBrace) && !IsAtEnd()) {
    block->stmts.push_back(ParseStmt());
  }
  Consume(TokenType::RBrace, "expect '}' ");
  return block;
}

std::unique_ptr<Stmt> Parser::ParseEmit() {
  Consume(TokenType::KwEmit, "expect keyword emit");
  auto stmt = std::make_unique<EmitStmt>();
  Consume(TokenType::LParen, "expect '(' after 'emit' ");
  auto expr = std::make_unique<VarExpr>();
  // 暂时不支持向自己，或者多个machine列表发送消息
  // 比如emit(this,Unlock(404))不支持
  // emit(device[i],Unlock(404))不支持
  // 不然就要对于emit target单独parse
  expr->name = std::move(Consume(TokenType::Identifier, "expect emit target").lexeme);
  stmt->target = std::move(expr);
  Consume(TokenType::Comma, "expect comma after event name");
  stmt->invoke = ParseEventInvoke();
  Consume(TokenType::RParen, "expect ')' after emit arguments");
  Consume(TokenType::Semicolon, "expect ';' ");
  return stmt;
}

EventInvoke Parser::ParseEventInvoke() {
  EventInvoke invoke;
  invoke.event_name = Consume(TokenType::Identifier, "expect event name").lexeme;
  Consume(TokenType::LParen, "expect '(' ");
  if (!Check(TokenType::RParen)) {
    do {
      invoke.args.push_back(ParseExpr()); // 这里不用std::move，因为ParseExpr返回的就是临时对象,就是会移动进去
    } while (Match(TokenType::Comma));
  }
  Consume(TokenType::RParen, "expect ')' ");
  return invoke;
}
} // namespace flux