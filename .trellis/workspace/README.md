# Trellis Workspace

`.trellis/workspace/` stores session notes and reusable agent memory for
Arborchive.

Use this directory for:

- recurring local workflow notes;
- reusable investigation summaries;
- environment observations that help future sessions;
- non-sensitive reminders about validation, tooling, or review habits.

Do not store secrets, tokens, private credentials, or machine-specific paths
that are not useful to other maintainers.

Workspace notes are lower priority than `AGENTS.md`, `docs/agent_workflow.md`,
`.trellis/spec/`, and `docs/roadmap.md`. If a workspace note becomes a stable
rule, promote it into `.trellis/spec/` or `docs/` as appropriate.
