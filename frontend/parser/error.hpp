#pragma once
#include "../lexer/token.hpp"
#include <string>

namespace flux {

inline std::string BuildParseError(const Token& token, const std::string& msg) {
    return msg + " at " + std::to_string(token.line) + ":" + std::to_string(token.column);
}

} // namespace flux