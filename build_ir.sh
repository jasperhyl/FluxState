#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_BIN="${ROOT_DIR}/fluxstate_ir"

cd "${ROOT_DIR}"

clang++ -std=c++20 \
  frontend/main.cpp \
  frontend/lexer/lexer.cpp \
  frontend/lexer/token.cpp \
  frontend/parser/parser.cpp \
  frontend/sema/scope.cpp \
  frontend/sema/context.cpp \
  frontend/sema/expr_analyzer.cpp \
  frontend/sema/stmt_analyzer.cpp \
  frontend/sema/machine_analyzer.cpp \
  frontend/sema/sema.cpp \
  backend/lowering/lowering_context.cpp \
  backend/lowering/expr_lowerer.cpp \
  backend/lowering/stmt_lowerer.cpp \
  backend/lowering/machine_lowerer.cpp \
  backend/lowering/lowering.cpp \
  backend/irgen/irgen_context.cpp \
  backend/irgen/runtime_decls.cpp \
  backend/irgen/type_converter.cpp \
  backend/irgen/value_utils.cpp \
  backend/irgen/expr_irgen.cpp \
  backend/irgen/stmt_irgen.cpp \
  backend/irgen/machine_irgen.cpp \
  backend/irgen/irgen.cpp \
  -I. \
  $(llvm-config --cxxflags --ldflags --libs core) \
  -fexceptions \
  -o "${OUTPUT_BIN}"

echo "Built ${OUTPUT_BIN}"
echo "Generate IR with:"
echo "  ${OUTPUT_BIN} test/test.fs > test/test.ll"
