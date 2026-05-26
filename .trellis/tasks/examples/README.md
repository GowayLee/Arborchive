# Task Examples

This directory is for commit-safe, non-personal examples that show future
agents how to organize task context in Arborchive.

Examples here are templates. They are not runtime task state, personal
onboarding tasks, active session records, or Trellis CLI output. Do not include
usernames, local machine paths, tokens, private keys, API keys, session IDs, or
current-task status.

## What Belongs Here

Use examples to document the shape of a task directory that can be copied or
adapted by Codex, Claude, Antigravity, or another agent without depending on a
specific agent platform.

Recommended structure:

```text
.trellis/tasks/examples/<task-kind>/
  prd.md
  research.md
  validation.md
  summary.md
```

Small tasks can keep these sections in one file, but larger tasks should split
them so the task boundary, investigation, checks, and handoff remain easy to
review.

## File Roles

`prd.md` should describe the task goal, scope, non-goals, allowed files,
forbidden files, acceptance criteria, and validation plan. It should make the
task boundary clear before implementation starts.

`research.md` should record relevant codebase findings, architecture notes,
open questions, alternatives considered, and decisions. It should cite local
files or docs when useful, but it should not become a long-term rule document.

`validation.md` should list the commands, database checks, documentation
checks, or manual review steps that prove the task is complete. It should also
record skipped checks with reasons.

`summary.md` should capture the final review handoff: changed files, commands
run, verification result, risks, known limitations, and follow-ups.

## Governance Boundaries

Keep the context layers separate:

- `.trellis/spec/` stores stable long-term engineering rules, architecture
  boundaries, and verification protocols.
- `.trellis/tasks/` stores context for a single task.
- `.trellis/workspace/` stores session notes and reusable memory that may help
  future sessions but is not formal project governance.
- `docs/` remains the home for human-readable roadmaps, phase summaries, and
  long-form analysis documents.

If a task note becomes a stable rule, promote it into `.trellis/spec/` or
`docs/` as appropriate. If it is temporary session memory, keep it in
`.trellis/workspace/`.

## What Must Not Be Committed

Do not commit personal onboarding or runtime task directories, such as
developer-specific join tasks or active-session scratch directories.

Do not commit content containing:

- personal usernames or developer IDs;
- local absolute paths;
- tokens, passwords, API keys, private keys, or credentials;
- active session state;
- generated Trellis CLI runtime files;
- assumptions that require Trellis CLI hooks or `.trellis/scripts/`.

Examples should be plain Markdown and should work as guidance even when an
agent only reads the repository files directly.
