#pragma once
#include "error.hpp"
#include "../lexer/token.hpp"
#include <string>
namespace flux {
std::string BuildParseError(const Token &token, const std::string &msg) {
  return "Parse error token at " + std::to_string(token.line) + ":" + std::to_string(token.column) + msg + ", got" +
         token.lexeme + "(" + TokenTypeToString(token.type) + ")";
}

} // namespace flux