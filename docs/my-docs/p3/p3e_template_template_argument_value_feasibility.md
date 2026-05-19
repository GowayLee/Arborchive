# P3e Template-template Argument Value Feasibility

## Question

Can table 112 `template_template_argument_value` be safely implemented under
the current Arborchive constraints?

## Schema Evidence

From `docs/semmlecode.cpp.dbscheme`:

```
template_template_argument_value(
    int type_id: @usertype ref,
    int index: int ref,
    int arg_value: @expr ref
);
```

Key constraint: `arg_value` is `@expr ref`. The value must be an expression.

No comments or annotations in the dbscheme provide additional semantics.

From `docs/scheme_info.md`, the auto-generated description says "模板模板参数值"
(template template parameter value), with `arg_value` referencing `expr(id)`.

## CodeQL Evidence

The remote CodeQL repository could not be directly queried (no `gh` CLI
available, WebFetch returned 404s for the canonical paths). However, the
dbscheme definition is unambiguous: `arg_value` is `@expr ref`.

The table parallels `class_template_argument_value`,
`function_template_argument_value`, `variable_template_argument_value`, and
`concept_template_argument_value` — all of which record **value** (expression)
template arguments.

The companion table `template_template_argument` (111) records **type**
arguments (`arg_type: @type ref`).

## Clang AST Evidence

Minimal test case:

```cpp
template <typename T> struct Holder { T value; };
template <template <typename> class TT> struct UsesTemplate { TT<int> value; };
template struct UsesTemplate<Holder>;
```

AST dump (LLVM 19, `-Xclang -ast-dump`):

```
ClassTemplateSpecializationDecl ... struct UsesTemplate definition ...
  TemplateArgument template 'Holder'
    ClassTemplateDecl ... Holder
```

Findings:

1. **TemplateArgument kind**: `TemplateArgument::Template`. The argument is a
   template name (`Holder`), not an expression.
2. **Clang source node**: `ClassTemplateDecl *`. This is a **Decl**, not an
   `Expr *`.
3. **No source expression**: `TemplateArgument::getAsExpr()` returns `nullptr`
   for `TemplateArgument::Template` kind.
4. **No source expression from TemplateArgumentLoc**:
   `TemplateArgumentLoc::getSourceExpression()` is only meaningful for
   `TemplateArgument::Expression` kind.
5. **Existing helper `getTemplateArgumentSourceExpr`** returns `nullptr` for
   `TemplateArgument::Template` kind (falls through to `default: return nullptr`).

**There is no real `Expr*` source for template-template arguments in Clang's
AST.** The source is always `TemplateName` / `TemplateDecl` / `ClassTemplateDecl`.

## Arborchive Current Coverage

| Table | ID | Status | Content |
|-------|----|--------|---------|
| `template_template_instantiation` | 110 | done | `to` (@usertype) / `from` (@usertype) — links TemplateTemplateParmDecl usertype to concrete template usertype |
| `template_template_argument` | 111 | done | `type_id` (@usertype) / `index` / `arg_type` (@type) — records TYPE arguments for template-template parameters |
| `template_template_argument_value` | 112 | deferred | would need `arg_value` (@expr) for template-template VALUE arguments |

### Why 111 already covers the information

For template-template parameters, the "argument" is a template name (e.g.,
`Holder` in `UsesTemplate<Holder>`). This is intrinsically a **type-level**
entity, not a value-level expression. Table 111 correctly records this
through `arg_type: @type ref`, which references the concrete template's
`@usertype` via the type system.

Table 112, by its schema (`arg_value: @expr ref`), requires an `@expr`
representation. But there is no `Expr*` in the Clang AST for template-template
arguments, and no suitable `ExprKind` in Arborchive's enum for representing a
template name as an expression.

### Overlap

- 111 (`arg_type: @type ref`) already expresses the identity of the
  template-template argument through the type system.
- 112 (`arg_value: @expr ref`) would try to express the same information
  through the expression system — but the data source is a `TemplateDecl`,
  not an `Expr*`.
- Fabricating a synthetic `@expr` row for a `TemplateDecl` would violate the
  schema contract that `arg_value` references an expression.

## ExprKind Audit

The current `ExprKind` enum in `include/model/db/expr.h` has no kind suitable
for representing a template name:

- `CONCEPT_ID` (393): represents a `ConceptSpecializationExpr` — a concept
  specialization, not a template name.
- `NONTYPE_TEMPLATE_PARAMETER` (394): represents a `NonTypeTemplateParmDecl`
  materialized as `@expr` — a non-type template parameter, not a template name.
- `TYPE_ID` (92): represents a type used as an operand — a type, not a
  template name.

There is no `TEMPLATE_NAME`, `TEMPLATE_ID`, or similar kind in the official
dbscheme enum (which terminates at 393). Adding a project-specific kind for
template names as expressions would be semantically unfounded.

## Decision

**Continue deferred — do not implement 112.**

### Reasons

1. **No source `Expr*` in Clang AST**: template-template arguments are
   `TemplateArgument::Template` → `TemplateDecl *`, not `Expr *`.
2. **No suitable `ExprKind`**: the official dbscheme `@expr.kind` enum has no
   value for "template name as expression." Adding one would be an ad-hoc
   extension without schema support.
3. **Table 111 already covers the same information**: `template_template_argument`
   records `arg_type: @type ref` which correctly represents the template-template
   argument through the type system.
4. **Synthesizing an `@expr` from a `TemplateDecl` would be semantically
   incorrect**: it would misrepresent a declaration as an expression, violating
   the `@expr` domain contract.

## Deferred Rationale

`template_template_argument_value.arg_value` is `@expr ref`, but the Clang
AST source for template-template arguments is `TemplateName` /
`TemplateDecl`, not `Expr*`. The companion table
`template_template_argument` (111) already records the template-template
argument through `arg_type: @type ref` — the correct domain for a template
name. Writing a synthetic `@expr` for a `TemplateDecl` would be semantically
wrong and has no precedent in the official CodeQL extractor's expr kind
enumeration.

## Future Conditions for Implementation

112 could potentially be implemented if:

1. The CodeQL C++ extractor source code is available and shows a specific
   `ExprKind` value used for template names as expressions.
2. Or the CodeQL QL library shows a predicate that reads from
   `template_template_argument_value` with a clear semantic contract.
3. Or a newer version of the dbscheme adds a `@template_name` or similar
   `@expr` subtype.

Until one of these conditions is met, 112 remains correctly deferred.
