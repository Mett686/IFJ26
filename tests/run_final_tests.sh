#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

COMPILER="${COMPILER:-./compiler}"
INTERPRETER="${INTERPRETER:-./interpret}"

if ! command -v "$COMPILER" >/dev/null 2>&1; then
  echo "Compiler not found: $COMPILER" >&2
  exit 1
fi

if ! command -v "$INTERPRETER" >/dev/null 2>&1; then
  echo "Interpreter not found: $INTERPRETER" >&2
  exit 1
fi

tmp_code="$(mktemp)"
tmp_actual="$(mktemp)"
tmp_log="$(mktemp)"
cleanup() {
  rm -f "$tmp_code" "$tmp_actual" "$tmp_log"
}
trap cleanup EXIT

run_test() {
  local name="$1"
  local source="$2"
  local input="$3"
  local expected="$4"

  if ! "$COMPILER" "$source" >"$tmp_code" 2>"$tmp_log"; then
    cat "$tmp_log" >&2
    echo "FAIL $name: compiler failed" >&2
    return 1
  fi

  if ! "$INTERPRETER" "$tmp_code" <"$input" >"$tmp_actual" 2>"$tmp_log"; then
    cat "$tmp_log" >&2
    echo "FAIL $name: interpreter failed" >&2
    return 1
  fi

  if ! diff -u "$expected" "$tmp_actual"; then
    echo "FAIL $name: output differs" >&2
    return 1
  fi

  echo "PASS $name"
}

failures=0
test_count=0
shopt -s nullglob
sources=(tests/final_tests/*.scala)

if ((${#sources[@]} == 0)); then
  echo "No source tests found in tests/final_tests" >&2
  exit 1
fi

for source in "${sources[@]}"; do
  base="${source%.scala}"
  name="${base##*/}"
  input="${base}.in"
  expected="${base}.out"
  test_count=$((test_count + 1))

  if [[ ! -f "$input" || ! -f "$expected" ]]; then
    echo "FAIL $name: missing matching .in or .out fixture" >&2
    failures=$((failures + 1))
    continue
  fi

  if ! run_test "$name" "$source" "$input" "$expected"; then
    failures=$((failures + 1))
  fi
done

test_count=$((test_count + 1))
if ! run_test control_flow_negative \
  tests/final_tests/control_flow.scala \
  tests/final_tests/control_flow_negative.in \
  tests/final_tests/control_flow_negative.out; then
  failures=$((failures + 1))
fi

if ((failures > 0)); then
  printf '%d of %d tests failed\n' "$failures" "$test_count" >&2
  exit 1
fi

printf 'All %d tests passed\n' "$test_count"
