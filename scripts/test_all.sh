#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

JOBS="${JOBS:-8}"
OUT_DIR="$ROOT_DIR/tests/output"
MAKE_ENV=()
MAKE_ARGS=()
CASES=(
  "slight-case"
  "moderate-case"
  "intense-case"
  "macro/macro_basic"
  "macro/macro_complex"
  "macro/macro_func"
  "preprocessor-simple"
  "temp"
  "unit-tests/function/coroutine"
  "unit-tests/function/template"
  "unit-tests/function/throw-exceptions"
  "unit-tests/function/typedef"
  "unit-tests/namespace"
  "unit-tests/p5/hierarchy_case"
  "unit-tests/p5/layout_case"
  "unit-tests/p5/semantic_gaps_case"
  "unit-tests/p6/lambda_case"
  "unit-tests/p7/attribute_presence_case"
  "unit-tests/p7/attribute_arguments_case"
  "unit-tests/p7/attribute_owner_links_case"
  "unit-tests/p7/attribute_string_args_case"
  "unit-tests/p7/attribute_constant_args_case"
  "unit-tests/p7/attribute_expr_args_case"
  "unit-tests/p7/attribute_assume_aligned_expr_args_case"
  "unit-tests/p7/attribute_extra_value_args_case"
  "unit-tests/p8/initialization_case"
  "unit-tests/p9/constexpr_flow_case"
  "unit-tests/p10/expression_graph_case"
  "unit-tests/p11/casts_conversion_case"
  "unit-tests/p12/allocation_lifetime_case"
  "unit-tests/p13/metadata_comments_case"
)

if [[ -z "${LLVM_CONFIG:-}" ]]; then
  if [[ -x "/opt/homebrew/opt/llvm@19/bin/llvm-config" ]]; then
    LLVM_CONFIG="/opt/homebrew/opt/llvm@19/bin/llvm-config"
  elif command -v llvm-config-19 >/dev/null 2>&1; then
    LLVM_CONFIG="$(command -v llvm-config-19)"
  elif command -v llvm-config19 >/dev/null 2>&1; then
    LLVM_CONFIG="$(command -v llvm-config19)"
  fi
fi

if [[ -n "${LLVM_CONFIG:-}" ]]; then
  MAKE_ARGS+=("LLVM_CONFIG=${LLVM_CONFIG}")
fi

if [[ -z "${TOML_CFLAGS:-}" ]]; then
  if [[ -d "/opt/homebrew/opt/toml11/include" ]]; then
    TOML_CFLAGS="-I/opt/homebrew/opt/toml11/include"
  elif [[ -d "/opt/homebrew/include" ]]; then
    TOML_CFLAGS="-I/opt/homebrew/include"
  fi
fi

if [[ -n "${TOML_CFLAGS:-}" ]]; then
  MAKE_ARGS+=("TOML_CFLAGS=${TOML_CFLAGS}")
fi

if [[ -n "${LDFLAGS:-}" ]]; then
  SANITIZED_LDFLAGS=()
  for flag in ${LDFLAGS}; do
    if [[ "$flag" == "-L/opt/homebrew/opt/llvm/lib" ]]; then
      continue
    fi
    SANITIZED_LDFLAGS+=("$flag")
  done
  MAKE_ENV+=("LDFLAGS=${SANITIZED_LDFLAGS[*]-}")
fi

mkdir -p "$OUT_DIR"

echo "[test_all] Building debug target with ${JOBS} jobs"
env "${MAKE_ENV[@]}" make "${MAKE_ARGS[@]}" debug -j "$JOBS"

TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/arborchive-test-all.XXXXXX")"
trap 'rm -rf "$TMP_DIR"' EXIT

echo "[test_all] Checking CLI exit behavior"
"$ROOT_DIR/build/demo" --help >"$TMP_DIR/help.log" 2>&1
grep -Fq "Usage: arborchive" "$TMP_DIR/help.log"
"$ROOT_DIR/build/demo" --version >"$TMP_DIR/version.log" 2>&1
grep -Fq "Arborchive version" "$TMP_DIR/version.log"
if "$ROOT_DIR/build/demo" --g0-invalid-option >"$TMP_DIR/bogus.log" 2>&1; then
  echo "[test_all] Invalid CLI option unexpectedly succeeded" >&2
  exit 1
fi

printf 'int main( { return 0; }\n' >"$TMP_DIR/invalid.cc"
if "$ROOT_DIR/build/demo" \
  -c "$ROOT_DIR/config.example.toml" \
  -s "$TMP_DIR/invalid.cc" \
  -o "$TMP_DIR/invalid.db" >"$TMP_DIR/invalid.log" 2>&1; then
  echo "[test_all] Invalid C++ source unexpectedly succeeded" >&2
  exit 1
fi
invalid_finished="$(python3 - "$TMP_DIR/invalid.db" <<'PY'
import sqlite3
import sys

with sqlite3.connect(sys.argv[1]) as connection:
    print(connection.execute("select count(*) from compilation_finished").fetchone()[0])
PY
)"
if [[ "$invalid_finished" != "0" ]]; then
  echo "[test_all] Failed parse was incorrectly marked finished" >&2
  exit 1
fi

for case_name in "${CASES[@]}"; do
  src="$ROOT_DIR/tests/${case_name}.cc"
  db="$OUT_DIR/${case_name//\//-}.db"
  log="$TMP_DIR/${case_name//\//-}.log"
  config="$ROOT_DIR/config.example.toml"

  if [[ "$case_name" == "unit-tests/p9/constexpr_flow_case" ]]; then
    config="$ROOT_DIR/tests/config.cxx23.toml"
  elif [[ "$case_name" == "unit-tests/p13/metadata_comments_case" ]]; then
    config="$ROOT_DIR/tests/unit-tests/p13/config.toml"
  fi

  if [[ ! -f "$src" ]]; then
    echo "[test_all] Missing test source: $src" >&2
    exit 1
  fi

  rm -f "$db"
  echo "[test_all] Running $case_name -> $db"
  if ! "$ROOT_DIR/build/demo" \
      -c "$config" \
      -s "$src" \
      -o "$db" >"$log" 2>&1; then
    echo "[test_all] Extractor failed for $case_name" >&2
    tail -80 "$log" >&2
    exit 1
  fi

  if grep -Eq "fatal error:|Error while processing|Failed to process AST" "$log"; then
    echo "[test_all] Fatal Clang diagnostic found for $case_name" >&2
    grep -E "fatal error:|Error while processing|Failed to process AST" "$log" >&2
    exit 1
  fi

  if [[ ! -s "$db" ]]; then
    echo "[test_all] Expected non-empty database was not created: $db" >&2
    exit 1
  fi

  "$ROOT_DIR/scripts/assert_test_db.py" "$case_name" "$db"

  echo "[test_all] Summary for $case_name"
  "$ROOT_DIR/scripts/db_summary.py" "$db"
done

echo "[test_all] All checks passed."
