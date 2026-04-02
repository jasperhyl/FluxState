#ifndef LEXER_H
#define LEXER_H

#include <cstdio>
#include <string>

// Token types returned by gettok()
enum Token {
  tok_eof = -1,

  // keywords
  tok_state = -2,
  tok_event = -3,
  tok_emit = -4,
  tok_from = -5,
  tok_on = -6,
  tok_after = -7,
  tok_when = -8,
  tok_do = -9,
  tok_start_timer = -10,
  tok_cancel_timer = -11,
  tok_set = -12,
  tok_map = -13,
  tok_entry = -14,
  tok_exit = -15,
  tok_guard = -16,
  tok_action = -17,

  // literals
  tok_identifier = -18,
  tok_number = -19,
  tok_string = -20,

  // operators / punctuation
  tok_arrow = -21,   // "->"

  // comparison operators
  tok_eq = -22,      // "=="
  tok_ne = -23,      // "!="
  tok_le = -24,      // "<="
  tok_ge = -25,      // ">="
  tok_lt = -26,      // "<"
  tok_gt = -27,      // ">"

  // logical operators
  tok_and = -28,     // "&&"
  tok_or = -29,      // "||"

  // new keywords for state machine
  tok_machine = -30,
  tok_initial = -31,
  tok_bool = -32,
  tok_int = -33,
  tok_double = -34,
  tok_true = -35,
  tok_false = -36,
  tok_else = -37,
  tok_while = -38,
  tok_if = -39,
};

// Global variables filled by lexer
extern std::string IdentifierStr;  // for tok_identifier
extern double NumVal;              // for tok_number
extern std::string StringVal;      // for tok_string

// Set input file (default stdin)
void setInputFile(FILE *file);

// Lexer entry point
int gettok();

#endif // LEXER_H