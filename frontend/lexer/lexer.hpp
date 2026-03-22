#pragma once
#include "token.hpp"
#include <string>
#include <vector>
namespace flux {
class Lexer {
public:
  // 读取源程序
  explicit Lexer(std::string source);
  // 循环调用词法规则，识别关键字，标识符，字面量，符号，跳过空白和注释。
  // 以EOF token作为结束标记
  std::vector<Token> Tokenize();

private:
  // 判断是否已经读到末尾
  bool IsAtEnd() const;
  // 看当前字符，但不前进指针
  char Peek() const;
  // 看下一个字符，不前进
  char PeekNext() const;
  // 消费当前字符并前进
  char Advance();
  // 条件消费，符合expect的就advance,否则放回false
  bool Match(char expected);

  // 跳过空白字符
  void SkipWhitespaceAndComments();
  // 从当前位置读取一个标识符串，再判断是标识符还是关键字
  Token LexIdentifierOrKeyword(int line, int column);
  // 读取数字字面量
  Token LexNumber(int line, int column);
  // 读取字符串字面量
  Token LexString(int line, int column);
  // 统一的token构造器，把lexeme和token type 和 行列号，封装成token
  Token MakeToken(TokenType type, const std::string &lexeme, int line, int column) const;

  std::string _source;
  size_t _current = 0; //_source中当前被读取的字符的下标
  int _line = 1;
  int _column = 1;
};
} // namespace flux