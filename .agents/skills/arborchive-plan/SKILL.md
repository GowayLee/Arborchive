---
name: arborchive-plan
description: "Create a lightweight Arborchive implementation plan with scope, non-goals, allowed paths, forbidden paths, required specs, validation plan, and risks before editing."
---

# Arborchive Plan

Use this skill before implementation when the task needs a clear boundary. It
is a lightweight Arborchive-specific planning aid, not Trellis runtime
integration.

## Plan Fields

Write a concise plan that covers:

- task goal;
- scope;
- non-goals;
- allowed files or directories;
- forbidden files or directories;
- required specs;
- validation plan;
- expected risks.

## Optional Task Notes

For complex tasks, suggest a generic task context directory:

```text
.trellis/tasks/<task-slug>/
  prd.md
  research.md
  validation.md
  summary.md
```

These files are reusable task context for review. They are not personal
session state and should not store developer identity, local state, or active
session data.

## Boundaries

- Do not create personal onboarding task directories.
- Do not write developer or session state.
- Do not auto commit.
- Do not expand the task beyond the stated subsystem.
