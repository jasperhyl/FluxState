#include "lexer.hpp"
#include "error.hpp"
#include "token.hpp"
#include <string>
#include <unordered_map>
namespace flux {
namespace {
// 定义KeyWordMap
const std::unordered_map<std::string, TokenType> KeyWordMap = {
    {"machine", TokenType::KwMachine},
    {"state", TokenType::KwState},
    {"event", TokenType::KwEvent},
    {"transition", TokenType::KwTransition},
    {"guard", TokenType::KwGuard},
    {"timeout", TokenType::KwTimeout},
    {"from", TokenType::KwFrom},
    {"on", TokenType::KwOn},
    {"when", TokenType::KwWhen},
    {"do", TokenType::KwDo},
    {"after", TokenType::KwAfter},
    {"int", TokenType::KwInt},
    {"char", TokenType::KwChar},
    {"short", TokenType::KwShort},
    {"double", TokenType::KwDouble},
    {"float", TokenType::KwFloat},
    {"bool", TokenType::KwBool},
    {"true", TokenType::KwTrue},
    {"false", TokenType::KwFalse},
    {"string", TokenType::KwString},
    {"s", TokenType::KwS},
    {"m", TokenType::KwM},
    {"h", TokenType::KwH},
};
} // namespace

} // namespace flux