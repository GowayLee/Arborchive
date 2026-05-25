# Trellis-Style Agent Workflow

This file defines the Arborchive agent workflow layer inspired by Trellis-style
workflow/spec/tasks/workspace separation.

It does not introduce a Trellis runtime dependency, copy Trellis source code, or
replace the existing `docs/` hierarchy.

## Governance Map

- `AGENTS.md` is the top-level constitution and architectural rule source.
- `AGENT_WORKFLOW.md` is the execution protocol. In this checkout, the file is
  currently stored as `docs/agent_workflow.md`.
- `AGENTS_GUIDE.md` is legacy reference material and has the lowest
  documentation priority.
- `.trellis/spec/` stores reusable long-term engineering rules for agents.
- `.trellis/tasks/` stores task-specific context for a single work item.
- `.trellis/workspace/` stores session notes and reusable memory.
- `docs/` remains the place for human-readable roadmaps, phase summaries, and
  long-form analysis.

`.trellis/` is an agent context layer. It must not replace `docs/`.

## Phase 1: Orient

Before acting, load the governance and task context in this order:

1. Read `AGENTS.md`.
2. Read `AGENT_WORKFLOW.md` (`docs/agent_workflow.md` in this checkout).
3. Read `.trellis/spec/README.md`.
4. Read the relevant `.trellis/spec/arborchive/*.md` files for the task.
5. Read task-specific notes under `.trellis/tasks/` when present.
6. Read reusable notes under `.trellis/workspace/` when they are relevant.

For C++ behavior or schema work, also load the current roadmap, AST dispatch
flow, related processor ownership, and related model/table files before making
changes.

## Phase 2: Plan

Do not begin by editing when the task boundary is unclear.

- Write or update research notes first when the requested scope is ambiguous.
- For complex work, produce a PRD, checklist, or validation plan before
  implementation.
- Identify the semantic subsystem that owns the change.
- Explicitly list files or directories that are allowed to change.
- Explicitly list files or directories that are forbidden to change.
- Decide which verification commands and database checks will prove the change.

Planning is part of the work. A patch that cannot be isolated to a clear
subsystem should be split or researched further.

## Phase 3: Implement

Make only the changes that fit the task boundary.

- Keep patches small, reversible, and reviewable.
- Keep `ASTVisitor` thin and dispatch-oriented.
- Put semantic extraction logic in the corresponding processor or helper layer.
- Use explicit processor APIs instead of expanding `Visit*` orchestration.
- Keep roadmap phases isolated.
- If schema changes are required, document the CodeQL alignment relationship.
- Do not add tables merely because they are convenient.

Implementation should preserve architecture boundaries even when a local quick
patch looks faster.

## Phase 4: Verify

Verification is mandatory. A task without Verify is incomplete.

Choose validation based on the patch type:

- Run tests that cover the modified behavior.
- Run `scripts/test_all.sh` for normal code or schema patches.
- Run `python3 scripts/db_summary.py tests/output/<case>.db` when semantic
  output changes.
- Run SQLite table and sample-row queries for schema or extraction changes.
- Regenerate ORM instantiations before validation when schema/ORM files change.
- For docs-only changes, validate the changed links, paths, hierarchy claims,
  and git diff scope.

If a command cannot be run, state the exact reason and the residual risk.
Do not silently skip verification.

## Phase 5: Summarize

Every final task summary must include:

- Changed files.
- Commands run.
- Verification result.
- Risks or known limitations.
- Follow-ups, if any.

The summary should make review easy: explain what changed, how it was checked,
and what remains uncertain.
