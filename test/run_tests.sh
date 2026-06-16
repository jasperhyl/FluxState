#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COMPILER="${ROOT_DIR}/fluxstate_ir"
FIXTURE="${ROOT_DIR}/test/test.fs"
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/fluxstate-tests.XXXXXX")"

cleanup() {
  rm -rf "${TMP_DIR}"
}
trap cleanup EXIT

log() {
  printf '[test] %s\n' "$*"
}

fail() {
  printf '[fail] %s\n' "$*" >&2
  exit 1
}

assert_file_nonempty() {
  local path="$1"
  [[ -s "${path}" ]] || fail "expected non-empty file: ${path}"
}

assert_contains() {
  local path="$1"
  local needle="$2"
  rg --fixed-strings --quiet -- "${needle}" "${path}" || fail "expected '${needle}' in ${path}"
}

assert_command_fails() {
  local output="$1"
  shift
  if "$@" >"${output}" 2>&1; then
    fail "expected command to fail: $*"
  fi
}

if ! command -v rg >/dev/null 2>&1; then
  fail "ripgrep (rg) is required to run tests"
fi

if [[ "${SKIP_BUILD:-0}" != "1" ]]; then
  log "building compiler"
  "${ROOT_DIR}/build_ir.sh" >/dev/null
fi

[[ -x "${COMPILER}" ]] || fail "compiler is missing or not executable: ${COMPILER}"
[[ -f "${FIXTURE}" ]] || fail "fixture is missing: ${FIXTURE}"

log "emitting LLVM IR"
"${COMPILER}" --O0 "${FIXTURE}" >"${TMP_DIR}/program.ll"
assert_file_nonempty "${TMP_DIR}/program.ll"
assert_contains "${TMP_DIR}/program.ll" "define void @fs_module_init"
assert_contains "${TMP_DIR}/program.ll" "define i1 @fs_step_SyntaxCoverage"
assert_contains "${TMP_DIR}/program.ll" "define i1 @fs_step_Secondary"

log "emitting DOT graph"
"${COMPILER}" --dot "${FIXTURE}" >"${TMP_DIR}/program.dot"
assert_file_nonempty "${TMP_DIR}/program.dot"
assert_contains "${TMP_DIR}/program.dot" "digraph SyntaxCoverage"
assert_contains "${TMP_DIR}/program.dot" "Idle -> Armed"
assert_contains "${TMP_DIR}/program.dot" "digraph Secondary"

log "emitting Mermaid graph"
"${COMPILER}" --mermaid "${FIXTURE}" >"${TMP_DIR}/program.mmd"
assert_file_nonempty "${TMP_DIR}/program.mmd"
assert_contains "${TMP_DIR}/program.mmd" "stateDiagram-v2"
assert_contains "${TMP_DIR}/program.mmd" "Idle --> Armed"
assert_contains "${TMP_DIR}/program.mmd" "S0 --> S1"

log "emitting object file"
"${COMPILER}" --emit-obj "${TMP_DIR}/program.o" "${FIXTURE}"
assert_file_nonempty "${TMP_DIR}/program.o"

log "linking and running executable without initial events"
"${COMPILER}" --emit-exe "${TMP_DIR}/idle_program" --idle-timeout-ms 10 --max-runtime-ms 100 "${FIXTURE}"
"${TMP_DIR}/idle_program"

log "linking and running executable with injected events"
"${COMPILER}" \
  --emit-exe "${TMP_DIR}/injected_program" \
  --inject SyntaxCoverage:Ping \
  --inject SyntaxCoverage:Configure:7,2,1.5,8.0,A,true,configured \
  --idle-timeout-ms 20 \
  --max-runtime-ms 300 \
  "${FIXTURE}"
"${TMP_DIR}/injected_program"

log "checking semantic failure path"
cat >"${TMP_DIR}/invalid.fs" <<'FS'
machine Broken {
    state Idle;
    initial Missing;
}
FS
assert_command_fails "${TMP_DIR}/invalid.out" "${COMPILER}" "${TMP_DIR}/invalid.fs"
assert_contains "${TMP_DIR}/invalid.out" "sema error"
assert_contains "${TMP_DIR}/invalid.out" "initial state 'Missing' is not declared"

log "all tests passed"
