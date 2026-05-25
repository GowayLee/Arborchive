---
name: arborchive-schema-review
description: "Review Arborchive schema and CodeQL alignment changes for table category, counterpart semantics, extension rationale, validation query, migration impact, and tests."
---

# Arborchive Schema Review

Use this skill for schema, ORM, database model, or CodeQL alignment review.

## Required Reads

Read:

- `.trellis/spec/arborchive/schema-codeql-alignment.md`
- `.trellis/spec/arborchive/architecture.md`
- `.trellis/spec/arborchive/testing-and-verification.md`

## Review Checklist

For every schema change, check:

- CodeQL counterpart;
- Arbor-specific reason;
- table category: CodeQL-aligned, Arbor extension, or experimental-deferred;
- validation query;
- alignment risk;
- migration impact;
- affected tests or fixtures.

Also check:

- no undocumented table was added;
- Arbor extension names use `arbor_*` or have clear documentation;
- CodeQL-aligned semantics were not polluted by convenience-only design;
- deferred or experimental ideas did not enter production schema silently.

## Boundaries

- Do not suggest a new table only because implementation would be easier.
- Do not mix unrelated roadmap phases into schema work.
- Require database evidence for schema or semantic output changes.
