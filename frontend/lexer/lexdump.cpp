#include "lexer.hpp"
#include "token.hpp"

#include <fstream>
#include <io.h>
#include <iostream>
#include <sstream>
#include <string>

namespace {
std::string EscapeForDisplay(const std::string &s) {
  std::string out;
  out.reserve(s.size());
  for (char ch : s) {
    switch (ch) {
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    case '\\':
      out += "\\\\";
      break;
    case '"':
      out += "\\\"";
      break;
    default:
      out.push_back(ch);
      break;
    }
  }
  return out;
}

std::string ReadAllFromIstream(std::istream &in) {
  std::ostringstream oss;
  oss << in.rdbuf();
  return oss.str();
}

std::string ReadFileToString(const std::string &path) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file) {
    throw std::runtime_error("failed to open file: " + path);
  }
  return ReadAllFromIstream(file);
}
} // namespace

int main(int argc, char **argv) {
  try {
    std::string source;

    if (argc >= 2) {
      source = ReadFileToString(argv[1]);
    } else {
      const bool stdin_is_tty = _isatty(_fileno(stdin)) != 0;
      if (!stdin_is_tty) {
        source = ReadAllFromIstream(std::cin);
      }

      if (source.empty()) {
        source = R"(
// sample input
machine AccessControl {
  state Locked;
  state Unlocked;
  initial Locked;

  event CardScanned(string who);

  transition from Locked on CardScanned(who) when true -> Unlocked do {
    // actions go here
  };

  transition from Unlocked after 10s when true -> Locked do { };
}
)";
      }
    }

    flux::Lexer lexer(std::move(source));
    const auto tokens = lexer.Tokenize();

    for (const auto &t : tokens) {
      std::cout << t.line << ':' << t.column << ' ' << flux::TokenTypeToString(t.type) << "  \"" << EscapeForDisplay(t.lexeme)
                << "\"\n";
    }

    return 0;
  } catch (const std::exception &ex) {
    std::cerr << ex.what() << "\n";
    return 1;
  }
}
