# Processor Boundary

Processors are the owners of semantic extraction in Arborchive.

`ASTVisitor` should discover AST entrypoints and delegate. Processors and their
helpers should own the meaning, normalization, validation assumptions, and
database-facing facts for their semantic domain.

## Processor Responsibilities

- Own semantic extraction for the corresponding domain.
- Provide explicit orchestration APIs for new `Visit*` logic to call.
- Keep APIs testable and reviewable.
- Reuse existing key, location, cache, and storage conventions.
- Document cross-domain writes when they are necessary.
- Avoid hidden coupling between unrelated roadmap phases.
- Include validation queries or targeted tests for large new extraction
  behavior.

## Domain Guidance

Common semantic domains include:

- template processor behavior;
- type processor behavior;
- namespace processor behavior;
- lambda processor behavior;
- inheritance and layout processor/helper behavior;
- function, variable, statement, and expression processor behavior when
  applicable;
- attribute and initialization processor/helper behavior when applicable.

This list describes ownership domains, not a requirement to invent class names.
If a concrete processor does not exist yet, establish the boundary in a helper
or dominant subsystem owner before adding extraction logic.

## Cross-Domain Writes

Processors should avoid writing facts owned by another domain unless the
relationship is documented and verified. When cross-domain writes are necessary,
the patch should explain:

- why the current domain is the right owner;
- which table or model is affected;
- how the relationship aligns with CodeQL semantics;
- which validation query or test proves the behavior.

## API Shape

Processor APIs should make intent clear:

- pass Clang AST objects and context needed for extraction;
- return stable IDs or status only when callers need them;
- hide schema row construction inside the owning layer;
- keep unsupported or deferred semantics explicit.

Large extraction work should come with targeted fixtures or SQL evidence, not
only a successful build.
