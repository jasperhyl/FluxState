#include "Lexer.h"
#include <cctype>
#include <cstdlib>
#include <iostream>

std::string IdentifierStr;
double NumVal;
std::string StringVal;

static FILE *InFile = stdin;
static int LastChar = ' ';

void setInputFile(FILE *file) {
  InFile = file;
  LastChar = getc(InFile);
}

static int nextChar() {
  return getc(InFile);
}

// Helper: skip whitespace and comments
static void skipWhitespaceAndComments() {
  while (true) {
    // skip whitespace
    while (isspace(LastChar))
      LastChar = nextChar();

    // skip comments starting with '#'
    if (LastChar == '#') {
      do
        LastChar = nextChar();
      while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
      continue;
    }

    // not whitespace or comment -> break
    break;
  }
}

// Helper: read an identifier (letter or underscore followed by alnum/underscore)
static int readIdentifier() {
  IdentifierStr = LastChar;
  while (isalnum((LastChar = nextChar())) || LastChar == '_')
    IdentifierStr += LastChar;

  // check keywords
  if (IdentifierStr == "state")       return tok_state;
  if (IdentifierStr == "event")       return tok_event;
  if (IdentifierStr == "emit")        return tok_emit;
  if (IdentifierStr == "from")        return tok_from;
  if (IdentifierStr == "on")          return tok_on;
  if (IdentifierStr == "after")       return tok_after;
  if (IdentifierStr == "when")        return tok_when;
  if (IdentifierStr == "do")          return tok_do;
  if (IdentifierStr == "start_timer") return tok_start_timer;
  if (IdentifierStr == "cancel_timer")return tok_cancel_timer;
  if (IdentifierStr == "set")         return tok_set;
  if (IdentifierStr == "map")         return tok_map;
  if (IdentifierStr == "entry")       return tok_entry;
  if (IdentifierStr == "exit")        return tok_exit;
  if (IdentifierStr == "guard")       return tok_guard;
  if (IdentifierStr == "action")      return tok_action;
  if (IdentifierStr == "machine")     return tok_machine;
  if (IdentifierStr == "initial")     return tok_initial;
  if (IdentifierStr == "bool")        return tok_bool;
  if (IdentifierStr == "int")         return tok_int;
  if (IdentifierStr == "double")      return tok_double;
  if (IdentifierStr == "true")        return tok_true;
  if (IdentifierStr == "false")       return tok_false;
  if (IdentifierStr == "else")        return tok_else;
  if (IdentifierStr == "while")       return tok_while;
  if (IdentifierStr == "if")          return tok_if;

  return tok_identifier;
}

// Helper: read a number (integer or floating point)
static int readNumber() {
  std::string NumStr;
  bool hasDot = false;

  do {
    if (LastChar == '.') {
      if (hasDot) break;  // second dot -> stop
      hasDot = true;
    }
    NumStr += LastChar;
    LastChar = nextChar();
  } while (isdigit(LastChar) || LastChar == '.');

  NumVal = strtod(NumStr.c_str(), nullptr);
  return tok_number;
}

// Helper: read a string literal enclosed in double quotes
static int readString() {
  StringVal.clear();
  LastChar = nextChar(); // consume opening "
  while (LastChar != '"' && LastChar != EOF && LastChar != '\n') {
    // simple: no escape sequences yet
    StringVal += LastChar;
    LastChar = nextChar();
  }
  if (LastChar == '"') {
    LastChar = nextChar(); // consume closing "
    return tok_string;
  }
  // unterminated string -> treat as error
  std::cerr << "Warning: unterminated string literal\n";
  return tok_string;
}

int gettok() {
  skipWhitespaceAndComments();

  if (LastChar == EOF)
    return tok_eof;

  // identifier or keyword
  if (isalpha(LastChar) || LastChar == '_')
    return readIdentifier();

  // number
  if (isdigit(LastChar) || LastChar == '.')
    return readNumber();

  // string literal
  if (LastChar == '"')
    return readString();

  // arrow: "->"
  if (LastChar == '-') {
    int next = nextChar();
    if (next == '>') {
      LastChar = nextChar();
      return tok_arrow;
    } else {
      LastChar = next;
      return '-';
    }
  }

  // logical operators: "&&" and "||"
  if (LastChar == '&') {
    int next = nextChar();
    if (next == '&') {
      LastChar = nextChar();
      return tok_and;
    }
    LastChar = next;
    return '&';
  }
  if (LastChar == '|') {
    int next = nextChar();
    if (next == '|') {
      LastChar = nextChar();
      return tok_or;
    }
    LastChar = next;
    return '|';
  }

  // comparison operators (==, !=, <=, >=, <, >)
  if (LastChar == '=') {
    int next = nextChar();
    if (next == '=') {
      LastChar = nextChar();
      return tok_eq;
    }
    LastChar = next;
    return '=';
  }
  if (LastChar == '!') {
    int next = nextChar();
    if (next == '=') {
      LastChar = nextChar();
      return tok_ne;
    }
    LastChar = next;
    return '!';
  }
  if (LastChar == '<') {
    int next = nextChar();
    if (next == '=') {
      LastChar = nextChar();
      return tok_le;
    }
    LastChar = next;
    return tok_lt;
  }
  if (LastChar == '>') {
    int next = nextChar();
    if (next == '=') {
      LastChar = nextChar();
      return tok_ge;
    }
    LastChar = next;
    return tok_gt;
  }

  // single-character tokens
  int ThisChar = LastChar;
  LastChar = nextChar();
  return ThisChar;
}