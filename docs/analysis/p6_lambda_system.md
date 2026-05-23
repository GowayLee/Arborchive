# P6 Lambda System

## Scope

P6 owns the lambda semantic subsystem only.

Target tables:

- `lambdas`
- `lambda_capture`

This phase extracts `clang::LambdaExpr` identity and capture metadata while
keeping `ASTVisitor` dispatch-only. Lambda-specific logic belongs in
`Lambda_Processor`.

## Safe Subset

This patch implements `lambdas` end-to-end:

- `expr`: the `@lambdaexpr` expression id, also recorded in `exprs` with
  `ExprKind::LAMBDAEXPR`.
- `default_capture`:
  - `none` for `LCD_None`
  - `by_copy` for `LCD_ByCopy`
  - `by_reference` for `LCD_ByRef`
- `has_explicit_return_type`: from Clang's `LambdaExpr::hasExplicitResultType`.

For `lambda_capture`, the safe subset records captures only when Clang exposes a
stable closure field mapping through `CXXRecordDecl::getCaptureFields`.
Supported rows include:

- capture index
- parent lambda expr id
- capture by-reference flag
- implicit/explicit flag
- capture source location
- mapped closure field as `@membervariable`

## Deferred Items

- `code_block` is excluded by the P6 roadmap boundary. Lambda body extraction,
  statement ownership, and body-to-routine relations belong to later control-flow
  or code-block work, not this lambda metadata patch.
- Lambda body statements are not extracted here.
- Expression graph parent/child relations are not extracted here.
- Initialization semantics are not extracted here.
- Attribute semantics are not extracted here.
- Closure type relation modeling is intentionally not expanded beyond the
  capture field needed by `lambda_capture.field`.
- Init-captures and VLA captures are deferred when no stable `FieldDecl` mapping
  is available from Clang. The patch does not insert placeholder or synthetic
  `@membervariable` ids.

## Capture Field Validation

`lambda_capture.field` references `@membervariable`. Clang stores captured
variables in closure-class fields and exposes a mapping via
`CXXRecordDecl::getCaptureFields`. That is a stable API for ordinary variable
captures and `this` captures, but Clang explicitly does not add entries for
init-captures. For any capture where no `FieldDecl` is available, Arborchive
skips the `lambda_capture` row rather than writing a bogus field id.

Manual validation should inspect both `lambda_capture` rows and the referenced
`membervariables` rows for the P6 testcase.
