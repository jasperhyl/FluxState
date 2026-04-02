#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include <memory>

// Current token
extern int CurTok;

// Token stream management
int getNextToken();

// Error reporting helpers
std::unique_ptr<ExprAST> LogError(const char *Str);
std::unique_ptr<StmtAST> LogErrorS(const char *Str);
std::unique_ptr<MachineAST> LogErrorM(const char *Str);

// Parsing functions
std::unique_ptr<ExprAST> ParseExpression();
std::unique_ptr<StmtAST> ParseStatement();
std::unique_ptr<StmtAST> ParseBlock();
std::unique_ptr<MachineAST> ParseMachine();

#endif // PARSER_H