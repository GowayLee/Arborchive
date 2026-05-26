# Testing and Verification

Verification is required for Arborchive patches. A task is not complete until
the agent reports what was run, what passed, what could not run, and what risk
remains.

## Common Commands

Run the full test entrypoint for normal code or schema patches:

```bash
scripts/test_all.sh
```

Inspect a generated database summary when semantic extraction changes:

```bash
python3 scripts/db_summary.py tests/output/<case>.db
```

List generated tables:

```bash
sqlite3 tests/output/<case>.db ".tables"
```

Inspect representative rows:

```bash
sqlite3 tests/output/<case>.db "select * from <table> limit 10;"
```

For schema/ORM changes, regenerate and rebuild before running tests:

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
```

## Validation by Patch Type

Docs-only changes:

- Validate that links and paths are accurate.
- Confirm the diff touches only allowed documentation paths.
- Report that behavior tests were not required when no code/schema changed.

C++ behavior changes:

- Run `scripts/test_all.sh`.
- Add or update targeted fixtures when the behavior is not covered.
- Use SQLite checks when output facts change.

Schema or ORM changes:

- Regenerate ORM instantiations.
- Build the debug target.
- Run the full test suite.
- Inspect `.tables`, row counts, and representative rows for affected tables.

Roadmap or governance changes:

- Check consistency with `AGENTS.md`, `docs/agent_workflow.md`, and
  `docs/roadmap.md`.
- Ensure `.trellis/` does not contradict or replace `docs/`.

## Reporting Format

Final summaries should include:

- changed files;
- validation commands;
- command results;
- unrun commands with reasons;
- remaining risks.

Never claim a task is fully verified if required commands could not run.
