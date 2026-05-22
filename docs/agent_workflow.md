# AGENT_WORKFLOW.md

Execution workflow for AI coding agents working on Arborchive.

Architecture governance is defined in `AGENTS.md`.
If this document conflicts with `AGENTS.md`, then:

```text
AGENTS.md wins.
```

This document defines execution flow, implementation discipline, and validation
protocol. It does not redefine architecture ownership.

---

# Phase 1 — Context Loading

Before modifying C++ behavior, agents MUST understand:

- current roadmap phase
- target semantic subsystem
- AST dispatch flow
- processor ownership
- related schema/model definitions

Minimum required context:

- `AGENTS.md`
- `docs/roadmap.md`
- `src/core/ast_visitor.cc`
- related processor/model/table files

If schema/ORM changes are involved, also review:

- `include/db/table_init.h`
- `include/db/storage_facade.h`
- `src/db/storage_facade.cc`
- `scripts/generate_instantiations.py`

---

# Phase 2 — Ownership Check

Before implementation, confirm:

- which semantic subsystem owns the task
- which processor/helper owns extraction logic
- whether existing ownership boundaries are still valid

Architectural target: `ASTVisitor` should stay as close as possible to
dispatch-only. Current legacy orchestration may exist; treat it as transitional
technical debt, not a pattern to expand.

If implementation requires:
- semantic orchestration in `Visit*`
- cross-table coordination in visitor code
- subsystem-specific branching in traversal logic

then:

```text
STOP and refactor ownership first.
```

Do not patch on top of architectural drift. If legacy visitor glue is involved,
keep changes compatibility-sized and avoid adding new ownership there.

## Architecture Drift Signals

These signals should trigger ownership review before implementation continues:

- `Visit*` methods grow too large
- repeated branching appears in visitor code
- multi-processor orchestration grows inside `ASTVisitor`
- new `STG`, `DependencyManager`, or `CacheManager` usage appears inside visitor code
- visitor code starts building schema rows, keys, dependencies, or cache policy
- a legacy orchestration-heavy `Visit*` needs more special cases

Ownership review means deciding whether logic belongs in an existing processor,
a processor helper, a dominant subsystem processor, or a future coordinator layer.

---

# Phase 3 — Processor Implementation

Implementation flow:

1. Identify AST entrypoints in `ASTVisitor`
2. Keep new `Visit*` methods thin and dispatch-oriented
3. Delegate semantic logic to processors/helpers
4. Reuse existing cache/key/location pipelines when possible
5. Insert DB rows through existing storage paths
6. Add isolated testcases when semantic behavior changes

Processor logic should own:
- extraction flow
- semantic coordination
- cache/key strategy
- deferred handling
- subsystem-specific logic

---

# Phase 4 — Schema / ORM Synchronization

If schema-related changes are required:

1. Update model definitions
2. Update table definitions
3. Update ORM/table initialization if needed
4. Update storage facade if required
5. Regenerate ORM instantiations

Required command:

```bash
python3 scripts/generate_instantiations.py
```

Schema changes must remain aligned with:

- `docs/datatable-list.txt`
- `docs/semmlecode.cpp.dbscheme`

Do not introduce ad-hoc schema naming.

---

# Phase 5 — Validation

Minimum validation after patches:

```bash
scripts/test_all.sh
```

If schema/ORM changed:

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
```

If semantic output changed:
- add isolated testcases when appropriate
- perform DB validation checks
- verify expected table population

Useful checks:

```bash
scripts/db_summary.py tests/output/intense-case.db
```

```bash
sqlite3 tests/output/intense-case.db \
'SELECT COUNT(*) FROM <table_name>;'
```

---

# Common Failure Signals

## LLVM Toolchain Mismatch

Symptoms:
- wrong LLVM version
- Apple Clang mismatch
- build failures during `make`

Check:

```bash
make print-toolchain
```

Arborchive currently targets LLVM 19.

---

## ORM / Instantiation Failures

Symptoms:
- missing template instantiations
- undefined storage methods
- ORM link failures

Check:
- model/table consistency
- storage facade updates
- ORM regeneration

Run:

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
```

---

## Empty Expected Tables

Symptoms:
- build succeeds
- tables exist
- expected rows are missing

Check:
- whether `ASTVisitor` is still dispatch-only
- processor ownership path
- cache/key early-return logic
- dependency/location failures before insertion

---

# Review Expectations

Every patch should clearly report:

- implementation plan
- changed files
- validation commands
- test results
- remaining risks

Small, reviewable, rollback-friendly patches are preferred.
