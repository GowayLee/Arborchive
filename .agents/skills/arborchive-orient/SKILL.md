---
name: arborchive-orient
description: "Orient an agent at the start of an Arborchive task by loading governance docs, Trellis-style context docs, and the relevant Arborchive specs before planning or editing."
---

# Arborchive Orient

Lightweight Arborchive-specific skill inspired by Trellis-style workflow. This
is not Trellis runtime integration.

## Required Reads

Read these files before planning or editing:

1. `AGENTS.md`
2. `docs/agent_workflow.md`
3. `.trellis/workflow.md`
4. `.trellis/spec/README.md`
5. Relevant `.trellis/spec/arborchive/*.md` files for the task

## Classify The Task

Classify the work before acting:

- docs-only
- roadmap or governance
- AST visitor boundary
- processor or semantic extraction
- schema or CodeQL alignment
- test or verification

## Output

Before implementation, state:

- task type;
- allowed files and directories;
- forbidden files and directories;
- specs read;
- remaining context still needed.

## Boundaries

- Do not edit code from this skill.
- Do not create generated task state.
- Do not rely on generated command layers.
