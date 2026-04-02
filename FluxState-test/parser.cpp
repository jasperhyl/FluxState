#include "Parser.h"
#include "Lexer.h"
#include <cstdio>
#include <cctype>
#include <map>

int CurTok;

static int GetTokPrecedence() {
  switch (CurTok) {
  case tok_eq: case tok_ne: case tok_le: case tok_ge: case '<': case '>':
  case tok_and: case tok_or:
    return 10;
  case '+': case '-': return 20;
  case '*': case '/': return 30;
  case '!': return 40;
  default: return -1;
  }
}

int getNextToken() {
  return CurTok = gettok();
}

std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "Parse error: %s\n", Str);
  return nullptr;
}
std::unique_ptr<StmtAST> LogErrorS(const char *Str) {
  fprintf(stderr, "Parse error: %s\n", Str);
  return nullptr;
}
std::unique_ptr<MachineAST> LogErrorM(const char *Str) {
  fprintf(stderr, "Parse error: %s\n", Str);
  return nullptr;
}

// ------------------------------------------------------------
// Expression parsing
// ------------------------------------------------------------
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (CurTok) {
  case tok_number: {
    double Val = NumVal;
    getNextToken();
    return std::make_unique<NumberExprAST>(Val);
  }
  case tok_string: {
    std::string Str = StringVal;
    getNextToken();
    return std::make_unique<StringExprAST>(Str);
  }
  case tok_identifier: {
    std::string Name = IdentifierStr;
    getNextToken();
    if (CurTok == '(') {
      getNextToken();
      std::vector<std::unique_ptr<ExprAST>> Args;
      if (CurTok != ')') {
        while (true) {
          auto Arg = ParseExpression();
          if (!Arg) return nullptr;
          Args.push_back(std::move(Arg));
          if (CurTok == ')') break;
          if (CurTok != ',') return LogError("Expected ')' or ',' in argument list");
          getNextToken();
        }
      }
      getNextToken();
      return std::make_unique<CallExprAST>(Name, std::move(Args));
    } else {
      return std::make_unique<VariableExprAST>(Name);
    }
  }
  case tok_true: {
    getNextToken();
    return std::make_unique<BoolExprAST>(true);
  }
  case tok_false: {
    getNextToken();
    return std::make_unique<BoolExprAST>(false);
  }
  case '(': {
    getNextToken();
    auto Expr = ParseExpression();
    if (!Expr) return nullptr;
    if (CurTok != ')') return LogError("Expected ')'");
    getNextToken();
    return Expr;
  }
  case '!': {
    getNextToken();
    auto Operand = ParsePrimary();
    if (!Operand) return nullptr;
    return std::make_unique<UnaryExprAST>('!', std::move(Operand));
  }
  default:
    return LogError("Expected expression");
  }
}

static std::unique_ptr<ExprAST> ParseBinOpRHS(int MinPrec, std::unique_ptr<ExprAST> LHS) {
  while (true) {
    int TokPrec = GetTokPrecedence();
    if (TokPrec < MinPrec)
      return LHS;
    int Op = CurTok;
    getNextToken();
    auto RHS = ParsePrimary();
    if (!RHS) return nullptr;
    LHS = std::make_unique<BinaryExprAST>(Op, std::move(LHS), std::move(RHS));
  }
}

std::unique_ptr<ExprAST> ParseExpression() {
  auto LHS = ParsePrimary();
  if (!LHS) return nullptr;
  return ParseBinOpRHS(0, std::move(LHS));
}

// ------------------------------------------------------------
// Statement parsing
// ------------------------------------------------------------
static std::unique_ptr<StmtAST> ParseEmitStmt() {
  getNextToken(); // eat 'emit'
  if (CurTok != tok_identifier) return LogErrorS("Expected event name after 'emit'");
  std::string EventName = IdentifierStr;
  getNextToken(); // eat event name
  if (CurTok != ';') return LogErrorS("Expected ';' after emit statement");
  getNextToken();
  return std::make_unique<EmitStmtAST>(EventName);
}

static std::unique_ptr<StmtAST> ParseIfStmt() {
  getNextToken(); // eat 'if'
  if (CurTok != '(') return LogErrorS("Expected '(' after 'if'");
  getNextToken();
  auto Cond = ParseExpression();
  if (!Cond) return nullptr;
  if (CurTok != ')') return LogErrorS("Expected ')' after condition");
  getNextToken();
  auto ThenStmt = ParseStatement();
  if (!ThenStmt) return nullptr;
  std::unique_ptr<StmtAST> ElseStmt = nullptr;
  if (CurTok == tok_else) {
    getNextToken();
    ElseStmt = ParseStatement();
    if (!ElseStmt) return nullptr;
  }
  return std::make_unique<IfStmtAST>(std::move(Cond), std::move(ThenStmt), std::move(ElseStmt));
}

static std::unique_ptr<StmtAST> ParseWhileStmt() {
  getNextToken(); // eat 'while'
  if (CurTok != '(') return LogErrorS("Expected '(' after 'while'");
  getNextToken();
  auto Cond = ParseExpression();
  if (!Cond) return nullptr;
  if (CurTok != ')') return LogErrorS("Expected ')' after condition");
  getNextToken();
  auto Body = ParseStatement();
  if (!Body) return nullptr;
  return std::make_unique<WhileStmtAST>(std::move(Cond), std::move(Body));
}

std::unique_ptr<StmtAST> ParseBlock() {
  getNextToken(); // eat '{'
  std::vector<std::unique_ptr<StmtAST>> Stmts;
  while (CurTok != '}' && CurTok != tok_eof) {
    auto Stmt = ParseStatement();
    if (!Stmt) return nullptr;
    Stmts.push_back(std::move(Stmt));
  }
  if (CurTok != '}') return LogErrorS("Expected '}'");
  getNextToken();
  return std::make_unique<BlockStmtAST>(std::move(Stmts));
}

std::unique_ptr<StmtAST> ParseStatement() {
  switch (CurTok) {
  case tok_emit:
    return ParseEmitStmt();
  case tok_if:
    return ParseIfStmt();
  case tok_while:
    return ParseWhileStmt();
  case '{':
    return ParseBlock();
  default: {
    // Try to parse an expression statement
    auto Expr = ParseExpression();
    if (!Expr)
      return LogErrorS("Expected expression statement");

    // Check if it's an assignment: variable '=' expression
    if (auto Var = dynamic_cast<VariableExprAST*>(Expr.get())) {
      if (CurTok == '=') {
        getNextToken(); // eat '='
        auto RHS = ParseExpression();
        if (!RHS) return nullptr;
        if (CurTok != ';')
          return LogErrorS("Expected ';' after assignment");
        getNextToken();
        return std::make_unique<AssignStmtAST>(Var->getName(), std::move(RHS));
      }
    }

    // Otherwise, it's a regular expression statement
    if (CurTok != ';')
      return LogErrorS("Expected ';' after expression");
    getNextToken();
    return std::make_unique<ExpressionStmtAST>(std::move(Expr));
  }
  }
}

// ------------------------------------------------------------
// Top-level machine parsing
// ------------------------------------------------------------
static std::unique_ptr<ExprAST> ParseOptionalGuard() {
  if (CurTok == tok_when) {
    getNextToken();
    auto Cond = ParseExpression();
    if (!Cond) return nullptr;
    return Cond;
  }
  return nullptr;
}

static std::unique_ptr<StmtAST> ParseOptionalDoBlock() {
  if (CurTok == tok_do) {
    getNextToken();
    if (CurTok != '{') {
      LogErrorS("Expected '{' after 'do'");
      return nullptr;
    }
    return ParseBlock();
  }
  return nullptr;
}

std::unique_ptr<MachineAST> ParseMachine() {
  if (CurTok != tok_machine) return LogErrorM("Expected 'machine'");
  getNextToken();
  if (CurTok != tok_identifier) return LogErrorM("Expected machine name");
  std::string MachineName = IdentifierStr;
  getNextToken();
  if (CurTok != '{') return LogErrorM("Expected '{'");
  getNextToken();

  std::vector<std::string> StateNames;
  std::string InitialState;
  std::vector<std::unique_ptr<VarDeclAST>> Variables;
  std::vector<std::unique_ptr<EventDeclAST>> Events;
  std::vector<std::unique_ptr<TransitionAST>> Transitions;

  while (CurTok != '}' && CurTok != tok_eof) {
    if (CurTok == tok_state) {
      getNextToken();
      std::vector<std::string> Names;
      do {
        if (CurTok != tok_identifier) return LogErrorM("Expected state name");
        Names.push_back(IdentifierStr);
        getNextToken();
        if (CurTok == ',') {
          getNextToken();
          continue;
        } else if (CurTok == ';') {
          break;
        } else {
          return LogErrorM("Expected ',' or ';' after state name");
        }
      } while (true);
      getNextToken();
      StateNames.insert(StateNames.end(), Names.begin(), Names.end());
    }
    else if (CurTok == tok_initial) {
      getNextToken();
      if (CurTok != tok_identifier) return LogErrorM("Expected state name after 'initial'");
      if (!InitialState.empty()) return LogErrorM("Multiple 'initial' declarations");
      InitialState = IdentifierStr;
      getNextToken();
      if (CurTok != ';') return LogErrorM("Expected ';' after initial state");
      getNextToken();
    }
    else if (CurTok == tok_bool || CurTok == tok_int || CurTok == tok_double) {
      std::string Type;
      if (CurTok == tok_bool) Type = "bool";
      else if (CurTok == tok_int) Type = "int";
      else Type = "double";
      getNextToken();
      if (CurTok != tok_identifier) return LogErrorM("Expected variable name");
      std::string VarName = IdentifierStr;
      getNextToken();
      std::unique_ptr<ExprAST> Init = nullptr;
      if (CurTok == '=') {
        getNextToken();
        Init = ParseExpression();
        if (!Init) return nullptr;
      }
      if (CurTok != ';') return LogErrorM("Expected ';' after variable declaration");
      getNextToken();
      Variables.push_back(std::make_unique<VarDeclAST>(Type, VarName, std::move(Init)));
    }
    else if (CurTok == tok_event) {
      getNextToken();
      if (CurTok != tok_identifier) return LogErrorM("Expected event name");
      std::string EventName = IdentifierStr;
      getNextToken();
      if (CurTok != '(') return LogErrorM("Expected '(' after event name");
      getNextToken();
      if (CurTok != ')') return LogErrorM("Events with parameters not supported");
      getNextToken();
      if (CurTok != ';') return LogErrorM("Expected ';' after event declaration");
      getNextToken();
      Events.push_back(std::make_unique<EventDeclAST>(EventName));
    }
    else if (CurTok == tok_from) {
      getNextToken();
      if (CurTok != tok_identifier) return LogErrorM("Expected source state");
      std::string Src = IdentifierStr;
      getNextToken();

      if (CurTok == tok_on) {
        getNextToken();
        if (CurTok != tok_identifier) return LogErrorM("Expected event name");
        std::string EventName = IdentifierStr;
        getNextToken();
        if (CurTok != '(') return LogErrorM("Expected '(' after event");
        getNextToken();
        if (CurTok != ')') return LogErrorM("Events with parameters not supported");
        getNextToken();
        if (CurTok != tok_arrow) return LogErrorM("Expected '->'");
        getNextToken();
        if (CurTok != tok_identifier) return LogErrorM("Expected destination state");
        std::string Dst = IdentifierStr;
        getNextToken();
        auto Guard = ParseOptionalGuard();
        auto Actions = ParseOptionalDoBlock();
        if (CurTok == ';') getNextToken();
        Transitions.push_back(std::make_unique<TransitionAST>(Src, EventName, Dst, std::move(Guard), std::move(Actions)));
      }
      else if (CurTok == tok_after) {
        getNextToken(); // eat 'after'
        auto NumExpr = ParsePrimary();
        if (!NumExpr) return nullptr;
        double Seconds = 0.0;
        if (auto Num = dynamic_cast<NumberExprAST*>(NumExpr.get()))
          Seconds = Num->Val;
        else
          return LogErrorM("Expected number after 'after'");
        if (CurTok != tok_identifier || IdentifierStr != "s")
          return LogErrorM("Expected 's' after number");
        getNextToken(); // eat 's'

        // Optional when condition
        std::unique_ptr<ExprAST> Guard = nullptr;
        if (CurTok == tok_when) {
          getNextToken(); // eat 'when'
          Guard = ParseExpression();
          if (!Guard) return nullptr;
        }

        if (CurTok != tok_arrow)
          return LogErrorM("Expected '->' after time/when condition");
        getNextToken(); // eat '->'

        if (CurTok != tok_identifier)
          return LogErrorM("Expected destination state name after '->'");
        std::string Dst = IdentifierStr;
        getNextToken();

        auto Actions = ParseOptionalDoBlock();
        if (CurTok == ';') getNextToken();

        Transitions.push_back(std::make_unique<TransitionAST>(Src, Seconds, Dst,
                                                              std::move(Guard),
                                                              std::move(Actions)));
      }
      else {
        return LogErrorM("Expected 'on' or 'after' after source state");
      }
    }
    else {
      return LogErrorM("Unexpected token in machine body");
    }
  }

  if (CurTok != '}') return LogErrorM("Expected '}'");
  getNextToken();

  if (InitialState.empty()) return LogErrorM("No initial state declared");
  bool found = false;
  for (const auto &s : StateNames) if (s == InitialState) { found = true; break; }
  if (!found) return LogErrorM("Initial state not declared in state list");

  return std::make_unique<MachineAST>(MachineName, std::move(StateNames), InitialState,
                                      std::move(Variables), std::move(Events), std::move(Transitions));
}