#include "parser.hpp"
#include "../ast/expr.hpp"
#include "../ast/machine.hpp"
#include "../ast/stmt.hpp"
#include "error.hpp"
#include <cctype>
#include <memory>
#include <stdexcept>

namespace flux {

// ---------- 辅助函数 ----------
static std::unique_ptr<Expr> MakeBinary(std::unique_ptr<Expr> lhs, BinaryOp op, std::unique_ptr<Expr> rhs) {
    auto node = std::make_unique<BinaryExpr>();
    node->lhs = std::move(lhs);
    node->op = op;
    node->rhs = std::move(rhs);
    return node;
}

Parser::Parser(std::vector<Token> tokens) : _tokens(std::move(tokens)) {}

Program Parser::ParseProgram() {
    Program program;
    while (!IsAtEnd()) {
        program.machines.push_back(ParseMachineDecl());
    }
    return program;
}

// ---------- Token 辅助 ----------
const Token& Parser::Peek() const { return _tokens[_current]; }
const Token& Parser::Previous() const { return _tokens[_current - 1]; }
bool Parser::IsAtEnd() const { return Peek().type == TokenType::EndOfFile; }
bool Parser::Check(TokenType type) const { return Peek().type == type; }
bool Parser::Match(TokenType type) {
    if (!Check(type)) return false;
    _current++;
    return true;
}
const Token& Parser::Consume(TokenType type, const std::string& msg) {
    if (!Check(type)) {
        throw std::runtime_error(BuildParseError(Peek(), msg));
    }
    _current++;
    return Previous();
}

// ---------- Machine 解析 ----------
MachineDecl Parser::ParseMachineDecl() {
    MachineDecl machine;
    Consume(TokenType::KwMachine, "expect 'machine'");
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
            machine.transitions.push_back(std::move(transition));
        } else if (Check(TokenType::KwInt) || Check(TokenType::KwShort) ||
                   Check(TokenType::KwFloat) || Check(TokenType::KwDouble) ||
                   Check(TokenType::KwChar) || Check(TokenType::KwBool) ||
                   Check(TokenType::KwString)) {
            // 全局变量声明
            auto var = ParseVarDecl();
            machine.vars.push_back(std::move(var));
        } else {
            throw std::runtime_error(BuildParseError(Peek(), "unexpected token in machine body"));
        }
    }
    Consume(TokenType::RBrace, "expect '}'");
    return machine;
}

VarDecl Parser::ParseVarDecl() {
    VarDecl var;
    var.type = ParseType();
    var.name = Consume(TokenType::Identifier, "expect variable name").lexeme;
    if (Match(TokenType::Assign)) {
        var.initializer = ParseExpr();
    }
    Consume(TokenType::Semicolon, "expect ';' after variable declaration");
    return var;
}

std::vector<std::string> Parser::ParseStateDecl(MachineDecl& /*machine*/) {
    Consume(TokenType::KwState, "expect keyword state");
    std::vector<std::string> states;
    do {
        states.push_back(Consume(TokenType::Identifier, "expect state name").lexeme);
    } while (Match(TokenType::Comma));
    Consume(TokenType::Semicolon, "expect ';' after state list");  // 添加
    return states;
}

std::string Parser::ParseInitialState(MachineDecl& /*machine*/) {
    Consume(TokenType::KwInitial, "expect keyword initial");
    std::string state = Consume(TokenType::Identifier, "expect initial state").lexeme;
    Consume(TokenType::Semicolon, "expect ';' after initial state");  // 添加
    return state;
}

EventDecl Parser::ParseEventDecl(MachineDecl& /*machine*/) {
    EventDecl event;
    Consume(TokenType::KwEvent, "expect keyword event");
    event.name = Consume(TokenType::Identifier, "expect event name").lexeme;
    Consume(TokenType::LParen, "expect '('");
    auto params = ParseParamList();  // 使用独立函数
    event.params = std::move(params);
    Consume(TokenType::RParen, "expect ')'");
    Consume(TokenType::Semicolon, "expect ';' after event declaration");
    return event;
}

ValueType Parser::ParseType() {
    if (Match(TokenType::KwInt)) return ValueType::Int;
    if (Match(TokenType::KwShort)) return ValueType::Short;
    if (Match(TokenType::KwFloat)) return ValueType::Float;
    if (Match(TokenType::KwDouble)) return ValueType::Double;
    if (Match(TokenType::KwChar)) return ValueType::Char;
    if (Match(TokenType::KwBool)) return ValueType::Bool;
    if (Match(TokenType::KwString)) return ValueType::String;
    throw std::runtime_error(BuildParseError(Peek(), "expect parameter type"));
}

std::vector<Param> Parser::ParseParamList() {
    std::vector<Param> params;
    // 如果下一个 token 是右括号，说明没有参数
    if (Check(TokenType::RParen)) {
        return params;
    }
    do {
        Param param;
        param.type = ParseType();
        param.name = Consume(TokenType::Identifier, "expect param name").lexeme;
        params.push_back(std::move(param));
    } while (Match(TokenType::Comma));
    return params;
}

// ---------- Transition ----------
TransitionDecl Parser::ParseTransitionDecl(MachineDecl& /*machine*/) {
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
        transition.guard = ParseGuard();
    }

    Consume(TokenType::Arrow, "expect '->'");
    transition.dst_state = Consume(TokenType::Identifier, "expect dst state").lexeme;

    if (Check(TokenType::KwDo)) {
    Consume(TokenType::KwDo, "expect 'do' keyword");
    transition.actions = ParseBlockStmt();
  }
    return transition;
}

TransitionTrigger Parser::ParseOnEventTrigger(TransitionDecl& /*transition*/) {
    Consume(TokenType::KwOn, "expect keyword on");
    OnEventTrigger trigger;
    trigger.event_name = Consume(TokenType::Identifier, "expect event name").lexeme;
    if (Match(TokenType::LParen)) {
        // 如果直接是右括号，则无绑定变量
        if (!Check(TokenType::RParen)) {
            do {
                trigger.bindings.push_back(Consume(TokenType::Identifier, "expect binding name").lexeme);
            } while (Match(TokenType::Comma));
        }
        Consume(TokenType::RParen, "expect ')'");
    }
    return trigger;
}

TransitionTrigger Parser::ParseAfterTrigger(TransitionDecl& /*transition*/) {
    Consume(TokenType::KwAfter, "expect keyword after");
    AfterTrigger trigger;
    auto numToken = Consume(TokenType::IntegerLiteral, "expect duration number");
    trigger.delay.duration = std::stoll(numToken.lexeme);
    if (Match(TokenType::KwS)) {
        trigger.delay.unit = DurationLiteral::Unit::Seconds;
    } else if (Match(TokenType::KwM)) {
        trigger.delay.unit = DurationLiteral::Unit::Minutes;
    } else if (Match(TokenType::KwH)) {
        trigger.delay.unit = DurationLiteral::Unit::Seconds; // 暂按秒
    } else {
        trigger.delay.unit = DurationLiteral::Unit::Seconds;
    }
    if (Match(TokenType::KwAfter)) {
        auto retryToken = Consume(TokenType::IntegerLiteral, "expect retry duration");
        trigger.retry = std::stoll(retryToken.lexeme);
        // 单位解析略
    }
    return trigger;
}

std::unique_ptr<Expr> Parser::ParseGuard() {
    Consume(TokenType::KwWhen, "expect keyword when");
    auto expr = ParseExpr();
    if (!expr) {
        throw std::runtime_error(BuildParseError(Peek(), "expect guard expr"));
    }
    return expr;
}

// ---------- 语句 ----------
std::unique_ptr<Stmt> Parser::ParseStmt() {
    if (Check(TokenType::LBrace)) {
        return ParseBlockStmt();
    }
    if (Check(TokenType::KwEmit)) {
        return ParseEmit();
    }
    return ParseExprStmt();
}

std::unique_ptr<BlockStmt> Parser::ParseBlockStmt() {
    Consume(TokenType::LBrace, "expect '{'");
    auto block = std::make_unique<BlockStmt>();
    while (!Check(TokenType::RBrace) && !IsAtEnd()) {
        block->stmts.push_back(ParseStmt());
    }
    Consume(TokenType::RBrace, "expect '}'");
    return block;
}

std::unique_ptr<ExprStmt> Parser::ParseExprStmt() {
    auto expr = ParseExpr();
    Consume(TokenType::Semicolon, "expect ';'");
    auto stmt = std::make_unique<ExprStmt>();
    stmt->expr = std::move(expr);
    return stmt;
}

std::unique_ptr<Stmt> Parser::ParseEmit() {
    Consume(TokenType::KwEmit, "expect keyword emit");
    Consume(TokenType::LParen, "expect '(' after 'emit'");
    auto stmt = std::make_unique<EmitStmt>();
    stmt->target = ParseExpr();
    Consume(TokenType::Comma, "expect ',' after target");
    stmt->invoke = ParseEventInvoke();
    Consume(TokenType::RParen, "expect ')' after emit arguments");
    Consume(TokenType::Semicolon, "expect ';'");
    return stmt;
}

EventInvoke Parser::ParseEventInvoke() {
    EventInvoke invoke;
    invoke.event_name = Consume(TokenType::Identifier, "expect event name").lexeme;
    Consume(TokenType::LParen, "expect '('");
    if (!Check(TokenType::RParen)) {
        do {
            invoke.args.push_back(ParseExpr());
        } while (Match(TokenType::Comma));
    }
    Consume(TokenType::RParen, "expect ')'");
    return invoke;
}

// ---------- 表达式 ----------
std::unique_ptr<Expr> Parser::ParseExpr() { return ParseAssign(); }

std::unique_ptr<Expr> Parser::ParseAssign() {
    auto lhs = ParseOr();
    if (Match(TokenType::Assign)) {
        auto rhs = ParseAssign();
        auto node = std::make_unique<AssignExpr>();
        node->lhs = std::move(lhs);
        node->rhs = std::move(rhs);
        return node;
    }
    return lhs;
}

std::unique_ptr<Expr> Parser::ParseOr() {
    auto lhs = ParseAnd();
    while (Match(TokenType::OrOr)) {
        auto rhs = ParseAnd();
        lhs = MakeBinary(std::move(lhs), BinaryOp::LogicOr, std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<Expr> Parser::ParseAnd() {
    auto lhs = ParseEquality();
    while (Match(TokenType::AndAnd)) {
        auto rhs = ParseEquality();
        lhs = MakeBinary(std::move(lhs), BinaryOp::LogicAnd, std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<Expr> Parser::ParseEquality() {
    auto lhs = ParseComparison();
    while (Match(TokenType::EqualEqual) || Match(TokenType::BangEqual)) {
        auto op = Previous().type;
        auto rhs = ParseComparison();
        lhs = MakeBinary(std::move(lhs), ParseBinaryOp(op), std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<Expr> Parser::ParseComparison() {
    auto lhs = ParseTerm();
    while (Match(TokenType::Less) || Match(TokenType::LessEqual) ||
           Match(TokenType::Greater) || Match(TokenType::GreaterEqual)) {
        auto op = Previous().type;
        auto rhs = ParseTerm();
        lhs = MakeBinary(std::move(lhs), ParseBinaryOp(op), std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<Expr> Parser::ParseTerm() {
    auto lhs = ParseFactor();
    while (Match(TokenType::Plus) || Match(TokenType::Minus)) {
        auto op = Previous().type;
        auto rhs = ParseFactor();
        lhs = MakeBinary(std::move(lhs), ParseBinaryOp(op), std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<Expr> Parser::ParseFactor() {
    auto lhs = ParseUnary();
    while (Match(TokenType::Star) || Match(TokenType::Slash)) {
        auto op = Previous().type;
        auto rhs = ParseUnary();
        lhs = MakeBinary(std::move(lhs), ParseBinaryOp(op), std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<Expr> Parser::ParseUnary() {
    if (Match(TokenType::Minus) || Match(TokenType::Increment) || Match(TokenType::Decrement)) {
        auto op = Previous().type;
        auto operand = ParseUnary();
        auto node = std::make_unique<UnaryExpr>();
        node->op = ParseUnaryOp(op);
        node->operand = std::move(operand);
        return node;
    }
    return ParsePrimary();
}

std::unique_ptr<Expr> Parser::ParsePrimary() {
    if (Match(TokenType::IntegerLiteral)) {
        auto expr = std::make_unique<IntLiteralExpr>();
        expr->value = std::stoi(Previous().lexeme);
        return expr;
    }
    if (Match(TokenType::StringLiteral)) {
        auto expr = std::make_unique<StringLiteralExpr>();
        expr->value = Previous().lexeme;
        return expr;
    }
    if (Match(TokenType::KwTrue)) {
        auto expr = std::make_unique<BoolLiteralExpr>();
        expr->value = true;
        return expr;
    }
    if (Match(TokenType::KwFalse)) {
        auto expr = std::make_unique<BoolLiteralExpr>();
        expr->value = false;
        return expr;
    }
    if (Match(TokenType::Identifier)) {
        std::string name = Previous().lexeme;
        // 检查是否为函数调用
        if (Match(TokenType::LParen)) {
            auto callee = std::make_unique<VarExpr>();
            callee->name = name;
            auto call = std::make_unique<CallExpr>();
            call->callee = std::move(callee);
            // 解析参数列表
            if (!Check(TokenType::RParen)) {
                do {
                    call->args.push_back(ParseExpr());
                } while (Match(TokenType::Comma));
            }
            Consume(TokenType::RParen, "expect ')' after arguments");
            return call;
        } else {
            auto expr = std::make_unique<VarExpr>();
            expr->name = name;
            return expr;
        }
    }
    if (Match(TokenType::LParen)) {
        auto expr = ParseExpr();
        Consume(TokenType::RParen, "expect ')'");
        return expr;
    }
    throw std::runtime_error(BuildParseError(Peek(), "expect primary expression"));
}

BinaryOp Parser::ParseBinaryOp(TokenType type) {
    switch (type) {
        case TokenType::AndAnd:     return BinaryOp::LogicAnd;
        case TokenType::OrOr:       return BinaryOp::LogicOr;
        case TokenType::Plus:       return BinaryOp::Add;
        case TokenType::Minus:      return BinaryOp::Sub;
        case TokenType::Star:       return BinaryOp::Mul;
        case TokenType::Slash:      return BinaryOp::Div;
        case TokenType::EqualEqual: return BinaryOp::Eq;
        case TokenType::BangEqual:  return BinaryOp::Ne;
        case TokenType::Less:       return BinaryOp::Lt;
        case TokenType::LessEqual:  return BinaryOp::Le;
        case TokenType::Greater:    return BinaryOp::Gt;
        case TokenType::GreaterEqual: return BinaryOp::Ge;
        default:
            throw std::runtime_error(BuildParseError(Peek(), "invalid binary operator"));
    }
}

UnaryOp Parser::ParseUnaryOp(TokenType type) {
    switch (type) {
        case TokenType::Minus:      return UnaryOp::Neg;
        case TokenType::Plus:       return UnaryOp::Plus;
        case TokenType::Star:       return UnaryOp::Deref;
        case TokenType::BitAnd:     return UnaryOp::AddressOf;  // 修正 Ampersand → BitAnd
        default:
            throw std::runtime_error(BuildParseError(Peek(), "invalid unary operator"));
    }
}

} // namespace flux