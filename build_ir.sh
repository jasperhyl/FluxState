#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_BIN="${ROOT_DIR}/fluxstate_ir"

CLANGXX_BIN="${CLANGXX_BIN:-}"
LLVM_CONFIG_BIN="${LLVM_CONFIG_BIN:-}"

if [[ -z "${CLANGXX_BIN}" ]]; then
  if command -v clang++-15 >/dev/null 2>&1; then
    CLANGXX_BIN="clang++-15"
  else
    CLANGXX_BIN="clang++"
  fi
fi

if [[ -z "${LLVM_CONFIG_BIN}" ]]; then
  if command -v llvm-config-15 >/dev/null 2>&1; then
    LLVM_CONFIG_BIN="llvm-config-15"
  else
    LLVM_CONFIG_BIN="llvm-config"
  fi
fi

cd "${ROOT_DIR}"

"${CLANGXX_BIN}" \
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
  backend/codegen/llvm_codegen.cpp \
  backend/linker/executable_builder.cpp \
  backend/optimizer/llvm_optimizer.cpp \
  backend/visualizer/visualizer.cpp \
  -I. \
  $("${LLVM_CONFIG_BIN}" --cxxflags --ldflags --libs core passes native nativecodegen --system-libs) \
  -std=c++20 \
  -fexceptions \
  -o "${OUTPUT_BIN}"

echo "Built ${OUTPUT_BIN}"
echo "Generate IR with:"
echo "  ${OUTPUT_BIN} test/test.fs > test/test.ll"
echo "Emit object with:"
echo "  ${OUTPUT_BIN} --emit-obj test/test.o test/test.fs"
echo "Emit executable with:"
echo "  ${OUTPUT_BIN} --emit-exe test/program --inject SyntaxCoverage:Ping test/test.fs"
