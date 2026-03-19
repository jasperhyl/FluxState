#pragma once
#include <string>
namespace flux {
enum class TokenType {

  EndOfFile,
  Identifier,
  IntegerLiteral,
  StringLiteral,

  // Punctuation
  LBrace,    // {
  RBrace,    // }
  LParen,    // (
  RParen,    // )
  Comma,     // ,
  Semicolon, // ;
  Arrow,     // ->
  Assign,    // =

  // Operators
  Plus,         // +
  Minus,        // -
  Star,         // *
  Slash,        // /
  Bang,         // !
  EqualEqual,   // ==
  BangEqual,    // !=
  Less,         // <
  LessEqual,    // <=
  Greater,      // >
  GreaterEqual, // >=
  AndAnd,       // &&
  OrOr,         // ||

  // Keywords
  KwMachine,
  KwState,
  KwInitial,
  KwEvent,
  KwTransition,
  KwGuard,
  KwTimeout,
  KwFrom,
  KwOn,
  KwWhen,
  KwAfter,
  KwDo,
  KwInt,
  KwShort,
  KwFloat,
  KwDouble,
  KwChar,
  KwBool,
  KwString,
  KwTrue,
  KwFalse,
  KwH,
  KwM,
  KwS, // time suffix: 10s -> IntegerLiteral(10), KwS
};
struct Token {
  TokenType type = TokenType::EndOfFile;
  std::string lexeme;
  int line = 1;
  int column = 1;
};
std::string TokenTypeToString(TokenType type);

} // namespace flux