#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

JOBS="${JOBS:-8}"
OUT_DIR="$ROOT_DIR/tests/output"
CASES=(
  "slight-case"
  "moderate-case"
  "intense-case"
  "unit-tests/namespace"
  "unit-tests/p5/hierarchy_case"
  "unit-tests/p5/layout_case"
)

mkdir -p "$OUT_DIR"

echo "[test_all] Building debug target with ${JOBS} jobs"
make debug -j "$JOBS"

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
