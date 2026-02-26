#!/usr/bin/env bash
set -euo pipefail

DB="${1:-tests/ast.db}"
shift || true

LIKE=""
COUNT_LIST=""
SAMPLES=()
FILE_NAME=""

usage() {
  cat <<EOF
Usage:
  $0 [db] [options]

Options:
  --like PATTERN             List tables matching pattern (e.g. preproc%).
  --count t1,t2,t3           Count rows for these tables (if table exists).
  --sample table:N           Print first N rows of table (repeatable).
  --file FILENAME            Focus: show preproc rows belonging to a file name in files.name
  -h, --help                 Show help.

Examples:
  $0 tests/ast.db
  $0 tests/ast.db --like preproc%
  $0 tests/ast.db --count functions,variable,types --sample functions:5
  $0 tests/ast.db --file macro_basic.cc
EOF
}

# ---------------- parse args ----------------
while [[ $# -gt 0 ]]; do
  case "$1" in
    --like) LIKE="${2:-}"; shift 2;;
    --count) COUNT_LIST="${2:-}"; shift 2;;
    --sample) SAMPLES+=("${2:-}"); shift 2;;
    --file) FILE_NAME="${2:-}"; shift 2;;
    -h|--help) usage; exit 0;;
    *) echo "[!] Unknown arg: $1"; usage; exit 1;;
  esac
done

if [[ ! -f "$DB" ]]; then
  echo "[!] DB not found: $DB"
  exit 1
fi

echo "[*] DB: $DB"

# helper: run single SQL and print
run_sql() {
  local sql="$1"
  sqlite3 "$DB" <<SQL
.headers on
.mode column
${sql}
SQL
}

# helper: check table exists (exit code 0/1)
table_exists() {
  local t="$1"
  sqlite3 "$DB" "SELECT 1 FROM sqlite_master WHERE type='table' AND name='$t' LIMIT 1;" | grep -q 1
}

echo
echo "== Basic summary =="
run_sql "
SELECT 'tables_total' AS k, COUNT(*) AS v FROM sqlite_master WHERE type='table';
SELECT name FROM sqlite_master WHERE type='table' ORDER BY name LIMIT 40;
"

# ---- default key tables we care about ----
DEFAULT_KEYS=(files locations functions variable types exprs stmts preprocdirects preproctext)

echo
echo "== Key table counts (only if exists) =="
for t in "${DEFAULT_KEYS[@]}"; do
  if table_exists "$t"; then
    n=$(sqlite3 "$DB" "SELECT COUNT(*) FROM \"$t\";")
    printf "%-20s %s\n" "$t" "$n"
  else
    printf "%-20s %s\n" "$t" "(missing)"
  fi
done

# ---- list tables by pattern ----
if [[ -n "$LIKE" ]]; then
  echo
  echo "== Tables LIKE '$LIKE' =="
  run_sql "SELECT name FROM sqlite_master WHERE type='table' AND name LIKE '$LIKE' ORDER BY name;"
fi

# ---- count specific list ----
if [[ -n "$COUNT_LIST" ]]; then
  echo
  echo "== Counts (requested) =="
  IFS=',' read -r -a arr <<<"$COUNT_LIST"
  for t in "${arr[@]}"; do
    t="${t// /}"
    [[ -z "$t" ]] && continue
    if table_exists "$t"; then
      n=$(sqlite3 "$DB" "SELECT COUNT(*) FROM \"$t\";")
      printf "%-20s %s\n" "$t" "$n"
    else
      printf "%-20s %s\n" "$t" "(missing)"
    fi
  done
fi

# ---- sample rows ----
if [[ ${#SAMPLES[@]} -gt 0 ]]; then
  echo
  echo "== Samples =="
  for item in "${SAMPLES[@]}"; do
    tbl="${item%%:*}"
    lim="${item##*:}"
    if [[ "$tbl" == "$lim" ]]; then lim="10"; fi
    if table_exists "$tbl"; then
      echo "-- $tbl (LIMIT $lim) --"
      run_sql "SELECT * FROM \"$tbl\" LIMIT $lim;"
    else
      echo "-- $tbl missing --"
    fi
  done
fi

# ---- focus by file ----
if [[ -n "$FILE_NAME" ]]; then
  echo
  echo "== Focus: file '$FILE_NAME' =="
  if ! table_exists "files"; then
    echo "[!] files table missing"
    exit 1
  fi

  file_id=$(sqlite3 "$DB" "SELECT id FROM files WHERE name='$FILE_NAME' LIMIT 1;")
  if [[ -z "$file_id" ]]; then
    echo "[!] file not found in files.name: $FILE_NAME"
    echo "    Try: sqlite3 $DB \"SELECT * FROM files LIMIT 20;\""
    exit 1
  fi
  echo "[*] file_id = $file_id"

  # need locations + preprocdirects
  if table_exists "locations" && table_exists "preprocdirects"; then
    echo
    echo "-- preprocdirects rows for this file (via location->locations.file) --"
    # NOTE: this assumes locations has column 'file'. If not, you’ll see error; then we adjust.
    run_sql "
SELECT p.id, p.kind, p.location
FROM preprocdirects p
JOIN locations l ON l.id = p.location
WHERE l.file = $file_id
LIMIT 50;
"
  else
    echo "[!] need locations + preprocdirects tables"
  fi

  # join preproctext if possible (common pattern: preprocdirects.id == preproctext.id)
  if table_exists "preproctext" && table_exists "locations" && table_exists "preprocdirects"; then
    echo
    echo "-- preproc text for this file (join p.id=t.id) --"
    run_sql "
SELECT l.line, l.column, p.kind, t.head, t.body
FROM preprocdirects p
JOIN preproctext t ON t.id = p.id
JOIN locations l ON l.id = p.location
WHERE l.file = $file_id
LIMIT 50;
"
  fi
fi

echo
echo "[+] Done."