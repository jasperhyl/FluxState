#pragma once
#include "../lexer/token.hpp"
#include <string>
namespace flux {
std::string BuildParseError(const Token &token, const std::string &msg);

}