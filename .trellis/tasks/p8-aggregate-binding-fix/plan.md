# P8 Aggregate Binding Fix Plan

## Goal and ownership

Repair `aggregate_field_init` for bases and designated union members without
changing the schema. `ExprProcessor` continues to own this relationship and
its deferred expression/field ID resolution. `ASTVisitor` stays unchanged.

## Context and boundaries

Reviewed governance, roadmap P8, the initialization task notes, model/table
definitions, and LLVM 19 `InitListExpr`/`RecursiveASTVisitor` headers.
Clang semantic initializers make subobjects explicit; base initializers precede
fields, unnamed bitfields consume no initializer, and unions expose the active
field through `getInitializedFieldInUnion()`.

Allowed changes: the aggregate path in `expr_processor.cc`, a P8 regression
fixture and exact semantic checker/mutation tests, the test runner hook, and
these task notes. `src/util/key_generator/expr.cc` may narrowly distinguish
InitListExpr nodes whose full source range collides with a child; the new
brace-elided single-base/deep-list tests prove that this prevents self edges.
Old databases must be regenerated; ordinary expression keys stay unchanged. No Visitor, schema, other roadmap phase, or unrelated
expression feature changes.

## Plan

1. Create a fixture covering one/multiple bases, ordinary and nested records,
   brace elision, union first/second selection, and unnamed bitfields.
2. Assert exact initializer-to-field/position identities using dynamically
   resolved IDs and source-backed values. Prove the old binary fails.
3. Separate initializer slot from field iteration; use the union's active field
   and skip unnamed bitfields. Resolve semantic form before consuming bindings.
4. Check deliberate wrong-field, wrong-value, wrong-position, missing-row, and
   extra-row mutations fail without changing the baseline database.
5. Build with LLVM19, run targeted tests and the full regression suite, retain
   baseline/modified/rollback evidence, and review the narrow diff.

## Contract and risks

`position` remains the semantic initializer sequence position (not a field's
declaration ordinal); base slots may leave a gap in the field-only relation.
Union's selected member uses its only initializer position, zero. Existing
deferred constructor/default-member initializer and expression families remain
separate. Implicit initializer/source-identity edge cases must be examined rather
than hidden by count-only assertions.
