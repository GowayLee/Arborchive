# Arborchive Architecture

Arborchive extracts C++ semantic facts from source code into a SQLite database
with CodeQL-compatible analysis as a major design constraint.

The intended flow is:

```text
source code
  -> Clang AST
  -> ASTVisitor traversal
  -> domain processors
  -> DB models
  -> SQLite output
  -> CodeQL-compatible analysis layer
```

## Layer Responsibilities

- `ASTVisitor` is traversal orchestration only.
- Processors own semantic extraction.
- DB models represent persisted facts.
- SQLite output is the reviewable artifact for extracted facts.
- CodeQL compatibility constrains naming, table semantics, and schema shape.

`ASTVisitor` may identify AST entrypoints and delegate work. It should not own
semantic pipelines, schema row assembly, or subsystem-specific extraction
policy.

Processors and helpers should own domain logic such as template semantics, type
extraction, namespace ownership, lambda capture extraction, inheritance/layout
facts, attributes, initialization, statements, and expressions.

## Schema Discipline

Schema changes must be documented and tied to a semantic subsystem. A schema
patch should explain:

- what CodeQL concept it aligns with;
- whether the table is a CodeQL counterpart, Arbor extension, or deferred
  experiment;
- which DB models, table definitions, and storage paths changed;
- how the output was validated.

Do not add tables only because they are convenient for an implementation path.
Do not silently break CodeQL compatibility.

## Verification Discipline

Behavior or schema changes require tests and SQLite validation. At minimum,
agents should run the relevant test suite and inspect the generated database
when extraction output changes.

Use targeted tests and SQL queries to show that representative rows exist and
that table semantics match the intended subsystem.

## Architectural Reminder

Do not optimize for quick patches at the cost of architecture boundaries. A
small patch that preserves ownership is better than a fast patch that turns
traversal, extraction, schema mapping, and storage policy into one tangled
change.
