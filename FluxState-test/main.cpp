#include "Lexer.h"
#include "Parser.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input.fs>\n";
    return 1;
  }

  FILE *input = fopen(argv[1], "r");
  if (!input) {
    std::cerr << "Cannot open file: " << argv[1] << "\n";
    return 1;
  }

  setInputFile(input);
  getNextToken();  // initialize CurTok
  auto machine = ParseMachine();
  fclose(input);

  if (!machine) {
    std::cerr << "Parsing failed.\n";
    return 1;
  }

  std::cout << "AST dump:\n";
  machine->dump();
  return 0;
}