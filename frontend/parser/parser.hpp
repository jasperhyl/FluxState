#pragma once

#include "../ast/machine.hpp"
#include "../lexer/token.hpp"
#include "error.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace flux {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    Program ParseProgram();

private:
    // machine 相关
    MachineDecl ParseMachineDecl();
    std::vector<std::string> ParseStateDecl(MachineDecl& machine);
    std::string ParseInitialState(MachineDecl& machine);
    EventDecl ParseEventDecl(MachineDecl& machine);
    std::vector<Param> ParseParamList();          // 改为独立函数
    ValueType ParseType();
    TransitionDecl ParseTransitionDecl(MachineDecl& machine);

    // trigger 解析
    TransitionTrigger ParseOnEventTrigger(TransitionDecl& transition);
    TransitionTrigger ParseAfterTrigger(TransitionDecl& transition);

    std::unique_ptr<Expr> ParseGuard();

    //变量声明
    VarDecl ParseVarDecl();

    // 语句
    std::unique_ptr<Stmt> ParseStmt();
    std::unique_ptr<BlockStmt> ParseBlockStmt();
    std::unique_ptr<ExprStmt> ParseExprStmt();
    std::unique_ptr<Stmt> ParseEmit();
    EventInvoke ParseEventInvoke();

    // 表达式
    std::unique_ptr<Expr> ParseExpr();
    std::unique_ptr<Expr> ParseAssign();
    std::unique_ptr<Expr> ParseOr();
    std::unique_ptr<Expr> ParseAnd();
    std::unique_ptr<Expr> ParseEquality();
    std::unique_ptr<Expr> ParseComparison();
    std::unique_ptr<Expr> ParseTerm();
    std::unique_ptr<Expr> ParseFactor();
    std::unique_ptr<Expr> ParseUnary();
    std::unique_ptr<Expr> ParsePrimary();

    BinaryOp ParseBinaryOp(TokenType type);
    UnaryOp ParseUnaryOp(TokenType type);

    // token 辅助
    const Token& Peek() const;
    const Token& Previous() const;
    bool IsAtEnd() const;
    bool Check(TokenType type) const;
    bool Match(TokenType type);
    const Token& Consume(TokenType type, const std::string& msg);

    std::vector<Token> _tokens;
    size_t _current = 0;
};

} // namespace flux