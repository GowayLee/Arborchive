---
name: arborchive-finish
description: "Finish an Arborchive task with a concise review-ready summary covering changed files, changes made, commands, verification result, risks, follow-ups, and commit message."
---

# Arborchive Finish

Use this skill when the work is ready to summarize. It is a reporting skill,
not an automation layer.

## Final Summary Fields

Include:

- changed files;
- what changed;
- commands run;
- verification result;
- risks or known limitations;
- follow-ups;
- suggested commit message.

For docs-only tasks, state that there is no runtime behavior change.

For code or schema tasks, state which tests and database checks were run. If a
command was not run, state why.

## Boundaries

- Do not auto commit.
- Do not push.
- Do not archive task directories.
- Do not write personal journals.
- Do not modify `.trellis/spec/` unless the user explicitly asks to preserve a
  long-term rule.
