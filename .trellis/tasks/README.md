# Trellis Tasks

`.trellis/tasks/` stores task-specific context for a single work item.

Use this directory for:

- research notes for an active task;
- PRDs or implementation checklists;
- validation plans;
- temporary handoff notes;
- task-specific SQL snippets or review evidence summaries.

Do not use this directory for stable architecture rules. Durable rules belong
in `.trellis/spec/`. Human-readable roadmap or phase analysis belongs in
`docs/`.

Task notes should be scoped, dated when useful, and removable after they are
captured in a PR, roadmap note, or long-term spec.

Suggested structure:

```text
.trellis/tasks/<task-slug>/
  plan.md
  research.md
  validation.md
```

For small tasks, a dedicated subdirectory is optional. Avoid turning task notes
into a second documentation tree.
