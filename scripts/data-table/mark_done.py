#!/usr/bin/env python3
"""
Mark tables that exist in SQLite database as 'done' in datatable-list.txt
"""

import sqlite3
import re
from pathlib import Path

# Paths
script_dir = Path(__file__).parent
project_root = script_dir.parent.parent
db_file = project_root / "tests" / "ast.db"
input_file = project_root / "docs" / "datatable-list.txt"
output_file = project_root / "docs" / "datatable-list.txt"

def get_db_tables(db_path):
    """Get all table names from SQLite database"""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;")
    tables = {row[0] for row in cursor.fetchall()}
    conn.close()
    return tables

def parse_datatable_list(file_path):
    """Parse datatable list into (index, name) tuples"""
    entries = []
    with open(file_path, "r") as f:
        for line in f:
            line = line.rstrip("\n")
            # Pattern: optional spaces, "index. tablename"
            match = re.match(r"^\s*(\d+)\.\s*(.+)$", line)
            if match:
                idx = int(match.group(1))
                name = match.group(2)
                entries.append((idx, name))
    return entries

def main():
    # Get implemented tables from database
    print(f"Reading tables from: {db_file}")
    db_tables = get_db_tables(db_file)
    print(f"Found {len(db_tables)} tables in database")

    # Parse datatable list
    print(f"Parsing: {input_file}")
    entries = parse_datatable_list(input_file)
    print(f"Found {len(entries)} entries in list")

    # Mark done entries and re-index
    output_lines = []
    done_count = 0
    for idx, name in entries:
        if name in db_tables:
            # Mark as done with checkmark
            padded_idx = str(idx).rjust(5)
            output_lines.append(f"DONE {padded_idx}. {name}")
            done_count += 1
        else:
            padded_idx = str(idx).rjust(5)
            output_lines.append(f"{padded_idx}. {name}")

    # Write output (overwrite original file)
    print(f"\nWriting to: {output_file}")
    with open(output_file, "w") as f:
        f.write("\n".join(output_lines) + "\n")

    print(f"\nSummary:")
    print(f"  Total entries: {len(entries)}")
    print(f"  Done (✓): {done_count}")
    print(f"  Remaining: {len(entries) - done_count}")

if __name__ == "__main__":
    main()
