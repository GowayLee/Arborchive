---
name: arborchive-verify
description: "Verify Arborchive changes using the project verification spec, choosing docs-only, behavior, schema, database, and build checks based on the change type."
---

# Arborchive Verify

Use this skill after changes and before final reporting. Follow
`.trellis/spec/arborchive/testing-and-verification.md`.

## Docs-Only Changes

Run:

```bash
git diff --check
git status --short
git diff --name-only
```

Explain why behavior tests are not required when no code, schema, build, or
fixture files changed.

## Behavior, Processor, Or Schema Changes

Run the relevant checks:

```bash
scripts/test_all.sh
```

When semantic output changes, inspect generated database output as needed:

```bash
python3 scripts/db_summary.py tests/output/<case>.db
sqlite3 tests/output/<case>.db ".tables"
sqlite3 tests/output/<case>.db "select * from <table> limit 10;"
```

For schema or ORM changes, consider the full path:

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
```

## Report

Always report:

- commands run;
- exit result;
- database files inspected;
- tables checked;
- commands not run, with reasons;
- remaining risk.

Do not claim completion when required checks fail or could not run.
