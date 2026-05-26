# Trellis Spec Layer

`.trellis/spec/` is the long-term rule layer for AI agents working on
Arborchive. It stores stable engineering constraints, architecture boundaries,
historical failure notes, and validation protocols.

It is not a task log. Temporary task summaries, scratch plans, or one-off
handoff notes belong in `.trellis/tasks/` or `.trellis/workspace/`.

## Governance Position

- `AGENTS.md` is the top-level constitution and total rule source.
- `AGENT_WORKFLOW.md` is the execution protocol. In this checkout, the file is
  currently stored as `docs/agent_workflow.md`.
- `AGENTS_GUIDE.md` is legacy reference material and has the lowest
  documentation priority.
- `.trellis/spec/` contains reusable long-term engineering rules for agents.
- `.trellis/tasks/` contains context for a single task.
- `.trellis/workspace/` contains session notes and reusable memory.
- `docs/` remains the place for human-readable roadmaps, phase summaries, and
  long-form analysis.

`.trellis/` complements `docs/`; it does not replace it.

## Arborchive Spec Index

- `arborchive/architecture.md`: high-level extraction architecture and
  persistent fact model.
- `arborchive/ast-visitor-boundary.md`: hard boundary for keeping
  `ASTVisitor` thin.
- `arborchive/processor-boundary.md`: processor ownership and semantic domain
  guidance.
- `arborchive/schema-codeql-alignment.md`: schema categories, CodeQL alignment,
  and extension rules.
- `arborchive/testing-and-verification.md`: validation commands and evidence
  expectations.
- `arborchive/commit-and-pr-style.md`: commit, PR, and review reporting style.

## Agent Usage

Before complex tasks, agents must read the specs relevant to the requested
subsystem. Schema tasks must read `schema-codeql-alignment.md`. Visitor or
processor tasks must read `ast-visitor-boundary.md` and
`processor-boundary.md`. Behavior-changing tasks must read
`testing-and-verification.md`.

Specs should record stable rules, architecture boundaries, historical mistakes,
and verification protocols. Do not put short-lived task conclusions here unless
they have become durable governance.
