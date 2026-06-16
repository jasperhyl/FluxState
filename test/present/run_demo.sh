#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DEMO_DIR="${ROOT_DIR}/test/present"
SOURCE="${DEMO_DIR}/smart_crossing.fs"
OUT_DIR="${DEMO_DIR}/out"
COMPILER="${ROOT_DIR}/fluxstate_ir"

mkdir -p "${OUT_DIR}"

if [[ "${SKIP_BUILD:-0}" != "1" ]]; then
  "${ROOT_DIR}/build_ir.sh" >/dev/null
fi

echo "[demo] emitting LLVM IR"
"${COMPILER}" --O2 "${SOURCE}" >"${OUT_DIR}/smart_crossing.ll"

echo "[demo] emitting DOT graph"
"${COMPILER}" --dot "${SOURCE}" >"${OUT_DIR}/smart_crossing.dot"

echo "[demo] emitting Mermaid graph"
"${COMPILER}" --mermaid "${SOURCE}" >"${OUT_DIR}/smart_crossing.mmd"

echo "[demo] emitting object file"
"${COMPILER}" --emit-obj "${OUT_DIR}/smart_crossing.o" "${SOURCE}"

echo "[demo] linking executable"
"${COMPILER}" \
  --emit-exe "${OUT_DIR}/smart_crossing" \
  --inject CrossingController:PedButton:101 \
  --idle-timeout-ms 50 \
  --max-runtime-ms 4000 \
  "${SOURCE}"

echo "[demo] running executable"
"${OUT_DIR}/smart_crossing"

echo "[demo] generated files:"
echo "  ${OUT_DIR}/smart_crossing.ll"
echo "  ${OUT_DIR}/smart_crossing.dot"
echo "  ${OUT_DIR}/smart_crossing.mmd"
echo "  ${OUT_DIR}/smart_crossing.o"
echo "  ${OUT_DIR}/smart_crossing"
