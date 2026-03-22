#include "lexer.hpp"
#include "error.hpp"
#include "token.hpp"
#include <cctype>
#include <stdexcept>
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
Lexer::Lexer(std::string source) : _source(std::move(source)) {}
std::vector<Token> Lexer::Tokenize() {
  std::vector<Token> tokens;
  while (!IsAtEnd()) {
    SkipWhitespaceAndComments();
    if (IsAtEnd()) {
      break;
    }

    const int token_line = _line;
    const int token_col = _column;
    const char c = Advance();
    // 识别是不是标识符或者keyword的起点
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
      // 在进入if之前，已经advance一次了，所以要回退一下，让处理函数从该单词的开头开始读取
      _current--;
      _column--;
      tokens.push_back(LexIdentifierOrKeyword(token_line, token_col));
      continue;
    }
    // 识别是不是数字
    if (std::isdigit(static_cast<unsigned char>(c))) {
      _current--;
      _column--;
      tokens.push_back(LexNumber(token_line, token_col));
      continue;
    }

    // 都不是的话，那只能是运算符号了
    switch (c) {
    case '{':
      tokens.push_back(MakeToken(TokenType::LBrace, "{", token_line, token_col));
      break;
    case '}':
      tokens.push_back(MakeToken(TokenType::RBrace, "}", token_line, token_col));
      break;
    case '(':
      tokens.push_back(MakeToken(TokenType::LParen, "(", token_line, token_col));
      break;
    case ')':
      tokens.push_back(MakeToken(TokenType::RParen, ")", token_line, token_col));
      break;
    case ',':
      tokens.push_back(MakeToken(TokenType::Comma, ",", token_line, token_col));
      break;
    case ';':
      tokens.push_back(MakeToken(TokenType::Semicolon, ";", token_line, token_col));
      break;
    case '+':
      if (Match('+')) {
        tokens.push_back(MakeToken(TokenType::Increment, "++", token_line, token_col));
      } else {
        tokens.push_back(MakeToken(TokenType::Plus, "+", token_line, token_col));
      }
      break;
    case '-':
      if (Match('>')) {
        tokens.push_back(MakeToken(TokenType::Arrow, "->", token_line, token_col));
      } else if (Match('-')) {
        tokens.push_back(MakeToken(TokenType::Decrement, "--", token_line, token_col));
      } else {
        tokens.push_back(MakeToken(TokenType::Minus, "-", token_line, token_col));
      }
      break;
    case '*':
      tokens.push_back(MakeToken(TokenType::Star, "*", token_line, token_col));
      break;
    case '/':
      tokens.push_back(MakeToken(TokenType::Slash, "/", token_line, token_col));
      break;
    case '!':
      if (Match('=')) {
        tokens.push_back(MakeToken(TokenType::BangEqual, "!=", token_line, token_col));
      } else {
        tokens.push_back(MakeToken(TokenType::Bang, "!", token_line, token_col));
      }
      break;
    case '=':
      if (Match('=')) {
        tokens.push_back(MakeToken(TokenType::EqualEqual, "==", token_line, token_col));
      } else {
        tokens.push_back(MakeToken(TokenType::Assign, "=", token_line, token_col));
      }
      break;
    case '<':
      if (Match('=')) {
        tokens.push_back(MakeToken(TokenType::LessEqual, "<=", token_line, token_col));
      } else {
        tokens.push_back(MakeToken(TokenType::Less, "<", token_line, token_col));
      }
      break;
    case '>':
      if (Match('=')) {
        tokens.push_back(MakeToken(TokenType::GreaterEqual, ">=", token_line, token_col));
      } else {
        tokens.push_back(MakeToken(TokenType::Greater, ">", token_line, token_col));
      }
      break;
    case '&':
      if (Match('&')) {
        tokens.push_back(MakeToken(TokenType::OrOr, "&&", token_line, token_col));
      } else {
        tokens.push_back(MakeToken(TokenType::BitAnd, "&", token_line, token_col));
      }
      break;
    case '|':
      if (Match('|')) {
        tokens.push_back(MakeToken(TokenType::OrOr, "||", token_line, token_col));
      } else {
        tokens.push_back(MakeToken(TokenType::BitOr, "|", token_line, token_col));
      }
      break;
    case '"':
      _current--;
      _column--;
      tokens.push_back(LexString(token_line, token_col));
      break;
    default:
      throw std::runtime_error(BuildError(std::string("unexpected character '") + c + "'", token_line, token_col));
    }
  }
  tokens.push_back(MakeToken(TokenType::EndOfFile, "", _line, _column));
  return tokens;
}

bool Lexer::IsAtEnd() const { return _current >= _source.size(); }

char Lexer::Peek() const { return IsAtEnd() ? '\0' : _source[_current]; }

char Lexer::PeekNext() const {
  if (_current + 1 >= _source.size()) {
    return '\0';
  }
  return _source[_current + 1];
}

char Lexer::Advance() {
  const char c = _source[_current++];
  if (c == '\n') {
    _line++;
    _column = 1;
  } else {
    _column++;
  }
  return c;
}

bool Lexer::Match(char expected) {
  if (IsAtEnd() || _source[_current] != expected) {
    return false;
  } else {
    return true;
  }
}

} // namespace flux