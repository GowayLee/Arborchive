# P3a Constraint Expr Investigation

## Scope

Investigate the Clang AST sources for concept definitions, requires expressions,
constrained type parameters, template requires clauses, and concept
specializations with non-type arguments. This pass does not change schema and
does not add table-writing logic for `type_template_type_constraint`,
`is_type_constraint`, or `concept_template_argument_value`.

## Fixtures

`tests/moderate-case.cc` now includes a minimal P3a fixture:

- `AlwaysTrue<T> = true`
- `HasPlusOne<T> = requires(T v) { v + 1; }`
- `template <AlwaysTrue T> struct ConstrainedParamBox`
- `template <typename T> requires HasPlusOne<T> struct RequiresClauseBox`
- `Positive<N> = (N > 0)` plus `requires Positive<N>`

The fixture avoids STL dependencies and instantiates each template with simple
builtin arguments.

## Observed AST Nodes

- Concept definitions are `ConceptDecl` nodes. Their initializer is a real
  `Expr*`: `CXXBoolLiteralExpr` for `AlwaysTrue`, `RequiresExpr` for
  `HasPlusOne`, and a parenthesized `BinaryOperator` for `Positive`.
- A constrained type parameter has a `TemplateTypeParmDecl` with a
  `TypeConstraint`. Clang exposes the immediately-declared constraint as a real
  `ConceptSpecializationExpr`.
- A template requires clause such as `requires HasPlusOne<T>` appears under the
  `ClassTemplateDecl` as a real `ConceptSpecializationExpr`.
- `requires Positive<N>` also appears as a real `ConceptSpecializationExpr`.
  Its template argument is an expression `DeclRefExpr` to the
  `NonTypeTemplateParmDecl`.

## Current Processor Behavior

- `ASTVisitor::VisitConceptDecl` records only `concept_templates`.
- `ASTVisitor::VisitConceptSpecializationExpr` records
  `concept_instantiation` and type-only `concept_template_argument` rows.
- `ExprProcessor` currently supports common expression nodes such as
  `DeclRefExpr`, literals, unary/binary operators, calls, implicit casts, array
  subscripts, init lists, and `sizeof`/`alignof`.
- `ExprProcessor` has no explicit `RequiresExpr` or
  `ConceptSpecializationExpr` handler, so those nodes can be traversed but are
  not safely materialized as `@expr` today.
- Template argument value extraction currently processes only safe literal
  source expressions for class/function/variable template arguments. Concept
  template argument values are not modeled yet.

## Safe Cases

- Concept templates can continue to be recorded from `ConceptDecl`.
- Type concept arguments in `ConceptSpecializationExpr`, such as
  `AlwaysTrue<int>` and `HasPlusOne<T>`, are safe for the existing
  `concept_template_argument` path when a type id can be resolved.
- Inner expressions inside a `RequiresExpr` body, such as `v + 1`, can be
  visited by existing `DeclRefExpr`, `IntegerLiteral`, and `BinaryOperator`
  handlers.
- The `Positive<N>` argument has a real source `DeclRefExpr`, but it is not a
  safe value argument for existing value extraction because the current helper
  only accepts literals.

## Deferred Cases

- `type_template_type_constraint` remains deferred because the current code does
  not bind a template type parameter usertype id to the immediately-declared
  constraint `Expr*`.
- `is_type_constraint` remains deferred because current concept ids do not
  distinguish ordinary concept specializations from type-constraint concept ids
  in a table-backed way.
- `concept_template_argument_value` remains deferred because there is no model
  or writer for concept value arguments, and `Positive<N>` would require
  handling a non-literal `DeclRefExpr` source expression without fabricating an
  expression.
- `RequiresExpr` and top-level `ConceptSpecializationExpr` should not be written
  to `@expr` until they have explicit `ExprKind` handling and stable cache/key
  behavior.

## Next Steps

P3b should first add explicit constraint extraction helpers around
`TemplateTypeParmDecl::getTypeConstraint()` and template associated constraints.
Only after `RequiresExpr` and `ConceptSpecializationExpr` have safe `@expr`
handling should P3b write `type_template_type_constraint` or
`is_type_constraint`. Concept value arguments need a separate safe-source check
that accepts real `Expr*` nodes such as non-type parameter `DeclRefExpr` without
pretending that `TemplateName` or `TemplateDecl` are expressions.
