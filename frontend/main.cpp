// main.cpp
#include "../backend/irgen/irgen.hpp"
#include "../backend/lowering/lowering.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "sema/diagnostics.hpp"
#include "sema/sema.hpp"
#include <fstream>
#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace flux;

namespace {

std::string ReadSourceFile(const char *path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("cannot open file: " + std::string(path));
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void PrintSemaDiagnostics(const std::vector<Diagnostic> &diagnostics) {
  for (const auto &diagnostic : diagnostics) {
    const char *level = diagnostic.level == Diagnostic::Level::Error ? "error" : "warning";
    std::cerr << "sema " << level << ": " << diagnostic.msg << '\n';
  }
}

void PrintStringDiagnostics(const char *stage, const std::vector<std::string> &diagnostics) {
  for (const auto &diagnostic : diagnostics) {
    std::cerr << stage << " error: " << diagnostic << '\n';
  }
}

} // namespace

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <source_file.fs>\n";
    return 1;
  }

  std::string source;
  try {
    source = ReadSourceFile(argv[1]);
  } catch (const std::runtime_error &error) {
    std::cerr << "io error: " << error.what() << '\n';
    return 1;
  }

  std::vector<Token> tokens;
  try {
    Lexer lexer(std::move(source));
    tokens = lexer.Tokenize();
  } catch (const std::runtime_error &error) {
    std::cerr << "lex error: " << error.what() << '\n';
    return 1;
  }

  Program program;
  try {
    Parser parser(std::move(tokens));
    program = parser.ParseProgram();
  } catch (const std::runtime_error &error) {
    std::cerr << "parse error: " << error.what() << '\n';
    return 1;
  }

  Sema sema;
  SemaResult sema_result = sema.Analyze(program);
  if (!sema_result.Ok()) {
    PrintSemaDiagnostics(sema_result.diagnostics);
    return 1;
  }

  Lowering lowering;
  LoweringResult lowering_result = lowering.Lower(program, sema_result.semantic_model);
  if (!lowering_result.Ok()) {
    PrintStringDiagnostics("lowering", lowering_result.diagnostics);
    return 1;
  }

  IRGen irgen;
  IRGenResult irgen_result = irgen.Generate(lowering_result.program, argv[1]);
  if (!irgen_result.Ok()) {
    PrintStringDiagnostics("irgen", irgen_result.diagnostics);
    return 1;
  }
  if (!irgen_result.module) {
    std::cerr << "irgen error: generated module is null\n";
    return 1;
  }

  std::string verify_error;
  llvm::raw_string_ostream verify_stream(verify_error);
  if (llvm::verifyModule(*irgen_result.module, &verify_stream)) {
    verify_stream.flush();
    std::cerr << "llvm verify error:\n" << verify_error;
    return 1;
  }

  irgen_result.module->print(llvm::outs(), nullptr);
  llvm::outs().flush();
  return 0;
}