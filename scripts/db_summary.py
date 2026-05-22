#!/usr/bin/env python3
"""Print a compact SQLite summary for Arborchive output databases."""

from __future__ import annotations

import argparse
import sqlite3
import sys
from pathlib import Path
from typing import Iterable


CORE_TABLES = [
    "functions",
    "fun_decls",
    "fun_def",
    "variable",
    "localvariables",
    "globalvariables",
    "membervariables",
    "params",
    "var_decls",
    "var_def",
    "types",
    "type_decls",
    "type_def",
    "builtintypes",
    "derivedtypes",
    "usertypes",
    "stmts",
    "exprs",
    "locations",
    "locations_default",
    "locations_stmt",
    "locations_expr",
    "containers",
    "files",
    "folders",
    "namespaces",
    "namespace_decls",
    "usings",
    "using_container",
    "declarations",
]

SUMMARY_TABLES = {
    "functions": ["id", "name", "kind"],
    "fun_decls": ["id", "function", "type_id", "name", "location"],
    "variable": ["id", "associate_id", "type"],
    "var_decls": ["id", "variable", "type_id", "name", "location"],
    "types": ["id", "associate_id", "type"],
    "builtintypes": ["id", "name", "kind", "size", "sign", "alignment"],
    "derivedtypes": ["id", "name", "kind", "type_id"],
    "usertypes": ["id", "name", "kind"],
    "stmts": ["id", "kind", "location"],
    "exprs": ["id", "kind", "location"],
    "locations": ["id", "associated_ed"],
    "locations_default": [
        "id",
        "container",
        "start_line",
        "start_column",
        "end_line",
        "end_column",
    ],
    "locations_stmt": [
        "id",
        "container",
        "start_line",
        "start_column",
        "end_line",
        "end_column",
    ],
    "locations_expr": [
        "id",
        "container",
        "start_line",
        "start_column",
        "end_line",
        "end_column",
    ],
    "namespaces": ["id", "name"],
    "namespace_decls": ["id", "namespace_id", "location", "bodylocation"],
    "usings": ["id", "element_id", "location", "kind"],
    "using_container": ["parent", "child"],
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Summarize Arborchive SQLite output."
    )
    parser.add_argument("db", help="Path to a generated SQLite database")
    parser.add_argument(
        "--limit",
        type=int,
        default=5,
        help="Rows to show for each summary table (default: 5)",
    )
    return parser.parse_args()


def quote_ident(name: str) -> str:
    return '"' + name.replace('"', '""') + '"'


def get_tables(conn: sqlite3.Connection) -> list[str]:
    rows = conn.execute(
        "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name"
    ).fetchall()
    return [row[0] for row in rows]


def get_columns(conn: sqlite3.Connection, table: str) -> list[str]:
    return [row[1] for row in conn.execute(f"PRAGMA table_info({quote_ident(table)})")]


def count_rows(conn: sqlite3.Connection, table: str) -> int:
    return int(conn.execute(f"SELECT COUNT(*) FROM {quote_ident(table)}").fetchone()[0])


def print_rows(headers: Iterable[str], rows: Iterable[sqlite3.Row]) -> None:
    headers = list(headers)
    print("  " + " | ".join(headers))
    print("  " + " | ".join("---" for _ in headers))
    for row in rows:
        print("  " + " | ".join(str(row[h]) for h in headers))


def print_table_sample(
    conn: sqlite3.Connection, table: str, preferred_columns: list[str], limit: int
) -> None:
    existing = get_columns(conn, table)
    columns = [col for col in preferred_columns if col in existing]
    if not columns:
        columns = existing[:6]
    if not columns:
        print("  (no columns)")
        return

    select_cols = ", ".join(quote_ident(col) for col in columns)
    rows = conn.execute(
        f"SELECT {select_cols} FROM {quote_ident(table)} LIMIT ?", (limit,)
    ).fetchall()
    if not rows:
        print("  (no rows)")
        return
    print_rows(columns, rows)


def main() -> int:
    args = parse_args()
    db_path = Path(args.db)
    if not db_path.is_file():
        print(f"[db_summary] DB not found: {db_path}", file=sys.stderr)
        return 1

    conn = sqlite3.connect(str(db_path))
    conn.row_factory = sqlite3.Row

    try:
        tables = get_tables(conn)
        table_set = set(tables)

        print(f"DB: {db_path}")
        print(f"Table count: {len(tables)}")

        print("\nCore row counts:")
        for table in CORE_TABLES:
            if table in table_set:
                print(f"  {table}: {count_rows(conn, table)}")
            else:
                print(f"  {table}: (missing)")

        print("\nCore summaries:")
        for table, columns in SUMMARY_TABLES.items():
            if table not in table_set:
                continue
            print(f"\n[{table}]")
            print_table_sample(conn, table, columns, args.limit)
    finally:
        conn.close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
