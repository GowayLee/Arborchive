# P3d Value Template Argument Support

## Scope

P3d implements non-type/value template argument `@expr` extraction for two tables:

- **90 `nontype_template_parameters`**: marks a `NonTypeTemplateParmDecl` as an
  `@expr` (kind=394 / `NONTYPE_TEMPLATE_PARAMETER`).
- **117 `concept_template_argument_value`**: records value (non-type) arguments
  of concept specializations when a real source `Expr*` is available.

Table 112 `template_template_argument_value` remains deferred by design.

## Schema Semantics

From `docs/semmlecode.cpp.dbscheme`:

```
nontype_template_parameters(
    int id: @expr ref
);

concept_template_argument_value(
    int concept_id: @concept ref,
    int index: int ref,
    int arg_value: @expr ref
);
```

- `nontype_template_parameters.id`: references `@expr`. Each
  `NonTypeTemplateParmDecl` is materialized as an `exprs` row with
  kind=394 (`NONTYPE_TEMPLATE_PARAMETER`). The key includes source file,
  source location, decl context, depth, index, parameter name, and parameter
  type string.
- `concept_template_argument_value.arg_value`: references `@expr`. Only
  written when the template argument has a real source `Expr*`.

## Implemented Safe Subset

### 1. NonTypeTemplateParmDecl → @expr (Table 90)

- `NonTypeTemplateParmDecl` is now materialized as an `exprs` row with
  kind=`NONTYPE_TEMPLATE_PARAMETER` (394).
- Stable key: `nttp-{source_loc}-ctx-{context}-depth-{d}-idx-{i}-name-{n}-type-{t}`.
- Dedup: `std::unordered_set<int>` by expr ID.
- Model: `DbModel::NonTypeTemplateParameter` in `include/model/db/type.h`.
- Table defs: `TypeTableFn::nontype_template_parameters()`.

### 2. concept_template_argument_value literal + DeclRefExpr subset (Table 117)

- Extracts value template arguments from `ConceptSpecializationExpr`:
  - Prefers `getTemplateArgsAsWritten()` → `TemplateArgumentLoc` → source expr.
  - Falls back to `arg.getAsExpr()` for `TemplateArgument::Expression` kind.
- Supports:
  - **DeclRefExpr to NonTypeTemplateParmDecl** (e.g., `PositiveValue<N>`
    where `N` is the outer template parameter): processed as `VARACCESS` (84).
  - **IntegerLiteral, FloatingLiteral, CharacterLiteral, CXXBoolLiteralExpr**:
    already supported by existing literal extraction.
- `processDeclRef` now always creates an `exprs` row with `VARACCESS` kind,
  not only for `VarDecl` references.
- `resolveTemplateArgumentExprId` now handles `DeclRefExpr` via
  `processDeclRef`.
- Dedup: `std::unordered_set<std::string>` by concept_id:index:arg_value
  triple.

### 3. Test Fixtures

Added to `tests/moderate-case.cc`:

```cpp
template <int N> struct ValueBox { int value; };
template <int N> concept PositiveValue = (N > 0);
template <int N> requires PositiveValue<N> struct PositiveValueBox {
  ValueBox<N> value;
};
ValueBox<1> valueBoxOne;
PositiveValueBox<1> positiveValueBoxOne;
```

## Deferred Cases

- **112 `template_template_argument_value`**: `arg_value` is `@expr` but
  template-template argument source is `TemplateName` / `TemplateDecl`, not
  `Expr*`. Cannot be written without schema or semantic changes.
- **SubstNonTypeTemplateParmExpr**: explicitly checked and deferred (no
  source expression identity).
- **Integral template arguments without source expr**: deferred (e.g.,
  `Positive<2>` where `2` is `TemplateArgument::Integral` with no
  `TemplateArgumentLoc` providing a source expression).
- **Alias templates**: not in scope.
- **Pack expansions**: not in scope.
- **Dependent/unresolved value arguments**: deferred.

## Files Changed

| File | Change |
|------|--------|
| `include/model/db/type.h` | Add `NonTypeTemplateParameter` struct |
| `include/model/db/concept.h` | Add `ConceptTemplateArgumentValue` struct |
| `include/model/db/expr.h` | Add `NONTYPE_TEMPLATE_PARAMETER = 394` to `ExprKind` |
| `include/db/table_defs/type.h` | Add `nontype_template_parameters()` |
| `include/db/table_defs/concept.h` | Add `concept_template_argument_value()` |
| `include/db/table_init.h` | Register both new tables |
| `include/core/processor/expr_processor.h` | Add `processNonTypeTemplateParmDecl`, remove `recordVarBindExpr` |
| `include/core/ast_visitor.h` | Add `VisitNonTypeTemplateParmDecl`, forward decl |
| `include/util/key_generator/expr.h` | Add `makeKeyForNonTypeTemplateParm` |
| `src/core/processor/expr_processor.cc` | Implement `processNonTypeTemplateParmDecl`, fix `processDeclRef` to always create expr row |
| `src/core/ast_visitor.cc` | Add `VisitNonTypeTemplateParmDecl`, `recordConceptTemplateArgumentValues`, dedup helpers, extend `resolveTemplateArgumentExprId` |
| `src/util/key_generator/expr.cc` | Implement `makeKeyForNonTypeTemplateParm` |
| `tests/moderate-case.cc` | Add P3d fixtures |

## Validation

```bash
git diff --check                              # passed
python3 scripts/generate_instantiations.py     # 127 instantiations
make debug -j 8                                # build OK
./scripts/test_all.sh                          # all passed
```

### SQLite Spot Checks

```
nontype_template_parameters: 9 rows
concept_template_argument_value: 2 rows
  - concept_id=1083 (CONCEPT_ID), index=0, arg_value=1088 (VARACCESS)
  - concept_id=1264 (CONCEPT_ID), index=0, arg_value=1269 (VARACCESS)
concept_template_argument (type args): 2 rows (existing, unchanged)
exprs with kind=394 (NONTYPE_TEMPLATE_PARAMETER): 9 rows
exprs with kind=393 (CONCEPT_ID): 7 rows
```

## Remaining Risks

1. `processDeclRef` now creates expr rows for all `DeclRefExpr` nodes
   (previously only for `VarDecl` refs). This creates extra `VARACCESS` rows
   for `FunctionDecl` and other non-VarDecl refs. These are harmless but
   increase exprs row count.
2. Integral non-type template arguments (e.g., `Positive<2>`) do not produce
   `concept_template_argument_value` rows because `TemplateArgument::Integral`
   has no source `Expr*`. This is semantically correct per the safe subset
   rule.
3. If downstream CodeQL tooling expects `concept_template_argument_value`
   for all non-type arguments (including integrals), a separate approach
   (e.g., synthesizing `IntegerLiteral` from APSInt) would be needed,
   but this is explicitly out of scope for P3d.
