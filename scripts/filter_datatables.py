#!/usr/bin/env python3
"""
Filter datatable-list.txt by removing entries starting with '@'
and re-indexing the remaining entries.
"""

import re
from pathlib import Path

# Input and output paths
script_dir = Path(__file__).parent
input_file = script_dir.parent / "docs" / "datatable-list.txt"
output_file = script_dir.parent / "docs" / "datatable-list-filtered.txt"

# Read the original file
with open(input_file, "r") as f:
    lines = f.readlines()

# Parse and filter entries
# Pattern: "index. name" where index is a number
pattern = re.compile(r"^(\d+)\.\s*(.+)$")
filtered_entries = []
removed_count = 0

for line in lines:
    line = line.rstrip("\n")
    match = pattern.match(line)
    if match:
        name = match.group(2)
        if not name.startswith("@"):
            filtered_entries.append(name)
        else:
            removed_count += 1

# Re-index and format output
output_lines = []
for idx, name in enumerate(filtered_entries, start=1):
    # Format: "idx. name" with idx right-aligned to 5 characters
    idx_str = str(idx)
    padded_idx = idx_str.rjust(5)
    output_lines.append(f"{padded_idx}. {name}")

# Write output
with open(output_file, "w") as f:
    f.write("\n".join(output_lines) + "\n")

print(f"Original entries: {len(lines)}")
print(f"Removed entries (starting with @): {removed_count}")
print(f"Remaining entries: {len(filtered_entries)}")
print(f"Output written to: {output_file}")
