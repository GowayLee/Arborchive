#!/usr/bin/env bash
set -euo pipefail

DB="${1:-tests/ast.db}"

echo "[*] DB: $DB"
sqlite3 "$DB" <<'SQL'
.headers on
.mode column

-- 1) 看主要表是否存在
SELECT name FROM sqlite_master WHERE type='table' ORDER BY name LIMIT 30;

-- 2) 关键表计数
SELECT COUNT(*) AS functions_cnt FROM functions;
SELECT COUNT(*) AS vars_cnt      FROM variable;
SELECT COUNT(*) AS types_cnt     FROM types;
SELECT COUNT(*) AS stmts_cnt     FROM stmt;
SELECT COUNT(*) AS exprs_cnt     FROM expr;

-- 3) 抽样看看内容（避免“表有但全空/字段异常”）
SELECT * FROM functions LIMIT 5;
SELECT * FROM variable  LIMIT 5;
SQL

echo "[+] Done."