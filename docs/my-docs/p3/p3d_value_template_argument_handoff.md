# P3d Value Template Argument Handoff

## Scope

P3d should only handle non-type/value template arguments when there is a real
source `Expr*` that can be safely materialized as `@expr`.

Do not handle `TemplateName` or `TemplateDecl` as expression values. Do not
implement `template_template_argument_value` unless the schema or source
semantics change, because template-template arguments are template names, not
source expressions.

## Current State

P3a completed the concept/constraint expression investigation:

- `ConceptDecl` initializer is a real `Expr*`.
- A constrained type parameter can be observed through
  `TemplateTypeParmDecl -> TypeConstraint -> ConceptSpecializationExpr`.
- Template requires clauses also expose `ConceptSpecializationExpr`.
- In `Positive<N>`, `N` is a real `DeclRefExpr`, but it was not included in
  the previous value extraction safe subset.

P3b completed the type constraint binding safe subset:

- `type_template_type_constraint` is implemented.
- `is_type_constraint` is implemented.
- `ConceptSpecializationExpr` is materialized as `exprs.kind = 393` /
  `CONCEPT_ID`.
- The `ConceptSpecializationExpr` key includes source file, canonical concept
  qualified name, and printed template argument text.

P3c completed the template-template instantiation safe subset:

- `TemplateTemplateParmDecl -> usertypes(kind = 8)` is implemented.
- `template_template_instantiation` is implemented.
- `from` is the `TemplateTemplateParmDecl` usertype with `kind = 8`.
- `to` is the concrete template usertype.
- `template_template_argument_value` remains correctly deferred because its
  `arg_value` is `@expr`, while `TemplateName` / `TemplateDecl` is not an
  `Expr`.

Current deferred tables relevant to P3d:

- 90 `nontype_template_parameters`
- 112 `template_template_argument_value`
- 117 `concept_template_argument_value`

## Target Tables

From `docs/datatable-list.txt`:

- 90 `nontype_template_parameters`
- 112 `template_template_argument_value`
- 117 `concept_template_argument_value`

From `docs/semmlecode.cpp.dbscheme`:

```text
nontype_template_parameters(
    int id: @expr ref
);

template_template_argument_value(
    int type_id: @usertype ref,
    int index: int ref,
    int arg_value: @expr ref
);

concept_template_argument_value(
    int concept_id: @concept ref,
    int index: int ref,
    int arg_value: @expr ref
);
```

P3d should focus on 90 and 117.

112 remains deferred unless schema or semantics change, because `arg_value` is
`@expr` but the source object for template-template arguments is a
`TemplateName` / `TemplateDecl`, not a real `Expr*`.

## Candidate Fixtures

Suggested minimal fixtures for Claude Code to add later, not in this handoff
patch:

```cpp
template <int N>
struct ValueBox {};

template <int N>
concept Positive = (N > 0);

template <int N>
requires Positive<N>
struct PositiveBox {};

ValueBox<1> vb1;
PositiveBox<1> pb1;
```

Optional, but should be added cautiously:

```cpp
template <int N>
requires Positive<N>
struct PositiveRefBox {};
```

Notes:

- Literal arguments may appear as `IntegerLiteral` or
  `SubstNonTypeTemplateParmExpr`.
- Parameter reference arguments may appear as `DeclRefExpr` to
  `NonTypeTemplateParmDecl`.
- Dependent or unresolved value arguments should remain deferred.

## Investigation Checklist for Claude Code

Read these files first:

- `docs/datatable-list.txt`
- `docs/semmlecode.cpp.dbscheme`
- `include/model/db/*`
- `include/db/table_defs/*`
- `src/core/ast_visitor.cc`
- `src/core/processor/expr_processor.cc`
- `src/core/processor/type_processor.cc`
- `src/util/key_generator/expr.cc`
- `src/util/key_generator/type.cc`
- `tests/moderate-case.cc`

Check these questions before implementation:

1. Does `NonTypeTemplateParmDecl` already have a stable id/model?
2. What object should `nontype_template_parameters.id` bind to?
3. Does `concept_template_argument_value.arg_value` reference `@expr`?
4. How can a real `Expr*` be obtained from non-type template arguments inside
   `ConceptSpecializationExpr`?
5. Does `TemplateArgumentLoc` or `TemplateArgument` preserve a source `Expr*`
   for the target cases?
6. Can the existing class/function/variable template argument value extraction
   helper be reused safely?
7. Can `DeclRefExpr` to `NonTypeTemplateParmDecl` be safely materialized as
   `@expr` with stable key/cache behavior?
8. Does `SubstNonTypeTemplateParmExpr` need an explicit handler, or should it
   remain deferred?

## Safe Implementation Rules

- Only write value tables when there is a real source `Expr*`.
- Do not fabricate expressions.
- Do not write `TemplateName` or `TemplateDecl` as `Expr*`.
- Prefer a literal-only safe subset first.
- Add a `DeclRefExpr` safe subset only after key/cache stability is confirmed.
- Defer dependent or unresolved arguments.
- Defer pack expansions.
- Defer alias/template-template argument values.

## Suggested Implementation Order

1. Do only investigation plus fixture.
2. Implement or confirm `NonTypeTemplateParmDecl -> model/id`.
3. Support `concept_template_argument_value` for a literal safe subset.
4. Then consider `DeclRefExpr` to `NonTypeTemplateParmDecl`.
5. Consider broader value expression support last.

## Validation Plan

Follow-up implementation should run:

```bash
git diff --check
python3 scripts/generate_instantiations.py
LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config CXX=/opt/homebrew/opt/llvm@19/bin/clang++ make debug -j 8
LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config CXX=/opt/homebrew/opt/llvm@19/bin/clang++ ./scripts/test_all.sh
```

Suggested SQLite spot checks:

```bash
sqlite3 tests/output/moderate-case.db ".tables" | tr ' ' '\n' | grep -E "nontype_template_parameters|concept_template_argument_value|template_argument_value|exprs"
sqlite3 tests/output/moderate-case.db "select count(*) from concept_template_argument_value;" 2>/dev/null || true
sqlite3 tests/output/moderate-case.db "select count(*) from nontype_template_parameters;" 2>/dev/null || true
sqlite3 tests/output/moderate-case.db "select id,kind from exprs order by id desc limit 20;" 2>/dev/null || true
```

## Handoff Notes

- P3d does not need to complete everything in one pass.
- If schema semantics are unclear, stop at investigation documentation.
- Do not revert or rewrite P3b/P3c.
- Do not expand into 112.
- Do not handle alias templates, packs, or dependent/unresolved cases in the
  first implementation pass.
