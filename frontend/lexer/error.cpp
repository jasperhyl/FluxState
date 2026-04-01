#include "error.hpp"
#include <string>
namespace flux {
std::string BuildError(const std::string &msg, int line, int column) {
  return "Lexer: Error at " + std::to_string(line) + ":" + std::to_string(column) + ": " + msg;
}

} // namespace flux