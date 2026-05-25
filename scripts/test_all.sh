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
  "unit-tests/namespace"
  "unit-tests/p5/hierarchy_case"
  "unit-tests/p5/layout_case"
  "unit-tests/p5/semantic_gaps_case"
  "unit-tests/p6/lambda_case"
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

for case_name in "${CASES[@]}"; do
  src="$ROOT_DIR/tests/${case_name}.cc"
  db="$OUT_DIR/${case_name//\//-}.db"

  if [[ ! -f "$src" ]]; then
    echo "[test_all] Missing test source: $src" >&2
    exit 1
  fi

  rm -f "$db"
  echo "[test_all] Running $case_name -> $db"
  "$ROOT_DIR/build/demo" \
    -c "$ROOT_DIR/config.example.toml" \
    -s "$src" \
    -o "$db"

  if [[ ! -s "$db" ]]; then
    echo "[test_all] Expected non-empty database was not created: $db" >&2
    exit 1
  fi

  echo "[test_all] Summary for $case_name"
  "$ROOT_DIR/scripts/db_summary.py" "$db"
done

echo "[test_all] All checks passed."
