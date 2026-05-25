# Schema and CodeQL Alignment

Arborchive schema work must preserve CodeQL compatibility as a first-class
constraint. Schema changes are semantic design work, not bookkeeping.

## Schema Change Categories

### 1. CodeQL-Aligned Tables

Use this category when the table is intended to correspond to a CodeQL C/C++
database concept.

- Preserve CodeQL naming and semantics where possible.
- Document the corresponding CodeQL concept.
- Keep column meanings aligned with `docs/semmlecode.cpp.dbscheme`.
- Validate representative rows against the intended CodeQL behavior.

### 2. Arbor Extension Tables

Use this category when Arborchive needs facts that do not have a direct CodeQL
counterpart.

- Use an `arbor_*` prefix or explicitly documented extension naming.
- Explain why CodeQL has no direct counterpart.
- Explain how the extension helps Arborchive without breaking CodeQL
  alignment.
- Make the extension boundary clear so future agents do not mistake it for a
  CodeQL table.

If a table is a reinforcement or extension rather than a CodeQL counterpart,
document it clearly. If naming may confuse CodeQL alignment, prefer explicit
`arbor_*` naming or equally explicit documentation.

### 3. Experimental or Deferred Tables

Use this category for planned or researched schema ideas that should not enter
the production schema yet.

- Record the decision in the roadmap or analysis docs.
- Do not silently add experimental tables to production schema.
- Explain why implementation is deferred.
- Record the condition that would make the table ready for implementation.

## Required Schema Change Explanation

Every schema change must state:

- CodeQL counterpart.
- Arbor-specific reason.
- Validation query.
- Alignment risk.
- Migration impact.
- Affected tests or fixtures.

## Hard Rules

- Do not silently break CodeQL compatibility.
- Do not add tables only because they are convenient.
- Do not use undocumented names that look CodeQL-aligned when they are Arbor
  extensions.
- Do not mix unrelated roadmap phases into a schema patch.
- Do not skip ORM regeneration when ORM/schema files change.

## Review Checklist

Before merging schema work, reviewers should be able to answer:

- Is this table listed in `docs/datatable-list.txt` or explicitly documented as
  an approved Arbor extension?
- Does the patch explain the CodeQL counterpart or lack of counterpart?
- Are model, table definition, initialization, and storage paths synchronized?
- Did the agent run generation and validation commands?
- Are SQL queries or DB summaries included for changed semantic output?
