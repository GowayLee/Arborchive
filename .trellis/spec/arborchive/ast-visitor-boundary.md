# ASTVisitor Boundary

`src/core/ast_visitor.cc` is a hard architecture boundary. The file should stay
as close as possible to a dispatch-only traversal layer.

## ASTVisitor May

- Visit Clang AST nodes.
- Preserve traversal order.
- Perform lightweight guards needed for traversal safety.
- Delegate to processors or helpers.
- Pass AST nodes and context information to processors.
- Keep `Visit*` functions thin and predictable.

## ASTVisitor Must Not

- Implement complex semantic extraction.
- Directly construct large business records.
- Directly insert database rows.
- Own template, type, layout, lambda, attribute, initialization, statement, or
  expression extraction details.
- Own CodeQL schema mapping decisions.
- Accumulate cache, key, dependency, or storage policy.
- Become a dumping ground for roadmap features.

## Preferred Shape

```cpp
bool ASTVisitor::VisitFooDecl(clang::FooDecl *D) {
  if (!D) return true;
  foo_processor_->processFooDecl(D);
  return true;
}
```

Short compatibility glue may exist in transitional legacy areas, but new work
should not expand that pattern. If a `Visit*` method needs repeated branching,
cross-table coordination, or schema-specific decisions, move the logic into an
owned processor or helper first.

## Historical Note

A previous roadmap mistake caused too much template-table logic to be placed
inside `ast_visitor.cc`. The project later refactored this logic back into
processors. Future agent work must treat ASTVisitor thinness as a hard
architectural boundary.

## Drift Signals

Stop and review ownership if:

- a new `Visit*` method calls multiple processors without a documented
  compatibility reason;
- visitor code assembles schema rows;
- visitor code directly uses database storage APIs;
- visitor code grows cache, key, or dependency policy;
- a roadmap feature is easier to add by placing subsystem logic in traversal.
