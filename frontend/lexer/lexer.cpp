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
const std::unordered_map<std::string, TokenType> KeywordMap = {
    {"machine", TokenType::KwMachine},
    {"state", TokenType::KwState},
  {"initial", TokenType::KwInitial},
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
        tokens.push_back(MakeToken(TokenType::AndAnd, "&&", token_line, token_col));
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
    Advance();
    return true;
  }
}

void Lexer::SkipWhitespaceAndComments() {
  while (!IsAtEnd()) {
    const char c = Peek();
    if (std::isspace(static_cast<unsigned char>(c))) {
      Advance();
      continue;
    }

    if (c == '/' && PeekNext() == '/') {
      Advance();
      Advance();
      while (!IsAtEnd() && Peek() != '\n') {
        Advance();
      }
      continue; // 让isspace消费掉最后的/n
    }

    if (c == '/' && PeekNext() == '*') {
      Advance();
      Advance();
      bool closed = false;
      while (!IsAtEnd()) {
        if (Peek() == '*' && PeekNext() == '/') {
          Advance();
          Advance();
          closed = true;
          break;
        }
        Advance();
      }
      if (closed == false) {
        throw std::runtime_error(BuildError("Unterminated block comment", _line, _column));
      }
      continue; // 当出现/*a*//*b*/需要continue再走一遍/*的判断逻辑
    }
    break;
  }
}

Token Lexer::LexIdentifierOrKeyword(int line, int column) {
  const size_t start = _current;
  // 先截取出来标识符或者关键字，再判断他是标识符还是关键字
  // 只要遇到不是字母，数字，下划线，就暂停截取
  // 读到_source的末尾也要暂停（即读到文件末尾也暂停）
  while (!IsAtEnd()) {
    const char c = Peek();
    if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
      Advance();
    } else {
      break;
    }
  }

  std::string lexeme = _source.substr(start, _current - start);
  const auto it = KeywordMap.find(lexeme);
  if (it != KeywordMap.end()) {
    return MakeToken(it->second, lexeme, line, column);
  }
  return MakeToken(TokenType::Identifier, lexeme, line, column);
}

Token Lexer::LexNumber(int line, int column) {
  const size_t start = _current;
  while (!IsAtEnd()) {
    if (std::isdigit(static_cast<unsigned char>(Peek()))) {
      Advance();
    } else {
      break;
    }
  }
  std::string lexeme = _source.substr(start, _current - start);
  return MakeToken(TokenType::IntegerLiteral, lexeme, line, column);
}

Token Lexer::LexString(int line, int column) {
  Advance(); // 消费掉引号
  std::string str;
  while (!IsAtEnd() && Peek() != '"') { // 退出while循环的时候，这两个只有可能其中一个不满足
    const char c = Advance();
    if (c == '\\') {
      // 这里使用严格转译，如果\后没有下列的转译字母，就build error
      if (IsAtEnd()) {
        throw std::runtime_error(BuildError("invalid escape sequence", line, column));
      }
      const char esc = Advance();
      switch (esc) {
      case 'n':
        str.push_back('\n');
        break;
      case 't':
        str.push_back('\t');
        break;
      case '"':
        str.push_back('"');
        break;
      case '\\':
        str.push_back('\\');
        break;
      default:
        throw std::runtime_error(BuildError("unsupported escape sequence", line, column));
      }
    } else {
      str.push_back(c);
    }
  }
  if (IsAtEnd()) {
    throw std::runtime_error(BuildError("unterminated string literal", line, column));
  }
  Advance(); // 消费右引号
  return MakeToken(TokenType::StringLiteral, str, line, column);
}

Token Lexer::MakeToken(TokenType type, const std::string &lexeme, int line, int column) const {
  Token token;
  token.type = type;
  token.lexeme = lexeme;
  token.line = line;
  token.column = column;
  return token;
}

} // namespace flux