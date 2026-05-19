# P3c Template-template Semantics

## Scope

P3c focuses on the safe subset for template-template parameters and direct
class-template arguments:

- `TemplateTemplateParmDecl` is modeled as `usertypes.kind = 8`.
- `TemplateArgument::Template` is recorded when it resolves directly to a
  `ClassTemplateDecl`.
- `template_template_instantiation` records the substitution edge from a
  template-template parameter to the concrete class template argument.

Alias templates, packs, dependent unresolved template names, substituted
template-template parameters that do not resolve to a direct `ClassTemplateDecl`,
and expression-valued template-template arguments remain deferred.

## Fixtures

`tests/moderate-case.cc` covers two no-STL cases:

- Existing mixed template-template plus type argument:
  `UsesTemplateTemplate<TTBox, int>`.
- Minimal explicit instantiation:
  `template struct UsesTemplate<Holder>;`.

The optional alias-template case is intentionally not added in this step.

## Schema Semantics

`docs/semmlecode.cpp.dbscheme` defines:

- `usertypes.kind = 8` as `@template_template_parameter`.
- `template_template_instantiation(to: @usertype, from: @usertype)`.
- `template_template_argument(type_id: @usertype, index: int, arg_type: @type)`.
- `template_template_argument_value(type_id: @usertype, index: int,
  arg_value: @expr)`.

For the safe subset, `template_template_instantiation` follows the same
`to/from` direction as the other `*_instantiation` tables:

- `to`: the concrete template usertype selected by the argument, for example
  the `Holder` or `TTBox` class-template usertype.
- `from`: the `TemplateTemplateParmDecl` usertype, for example `TT` or `C`
  with `kind = 8`.

`template_template_argument` continues to use the owning class specialization
as `type_id` and the concrete template usertype as `arg_type`.

## Observed AST Nodes

For `UsesTemplateTemplate<TTBox, int>`, Clang 19 reports:

- The primary template parameter list contains `TemplateTemplateParmDecl C`.
- The class specialization contains `TemplateArgument template 'TTBox'`.
- That argument resolves directly to a `ClassTemplateDecl TTBox`.

This gives a stable parameter id and a stable concrete template id without
turning `TemplateName` or `TemplateDecl` into an `Expr`.

## Implemented Safe Subset

- Added `TypeProcessor::processTemplateTemplateParmDecl`.
- Added `ASTVisitor::VisitTemplateTemplateParmDecl`.
- Added a stable key path for `TemplateTemplateParmDecl`, including name,
  depth, index, and source location.
- Added `template_template_instantiation` model/table/init support.
- Added recording for class template specializations only when:
  - the corresponding template parameter is a non-pack
    `TemplateTemplateParmDecl`;
  - the argument kind is `TemplateArgument::Template`;
  - the argument resolves directly to `ClassTemplateDecl`.

## Deferred Cases

- `TypeAliasTemplateDecl` arguments.
- Template-template parameter packs and pack expansions.
- Dependent or unresolved `TemplateName` forms.
- Substituted template-template parameter names that do not resolve directly to
  a `ClassTemplateDecl`.
- `template_template_argument_value`, because its `arg_value` column is
  `@expr`, and the current safe subset has `TemplateName` / `TemplateDecl`
  semantics rather than a real source `Expr`.

## Validation

Validated commands:

```bash
git diff --check
python3 scripts/generate_instantiations.py
LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config CXX=/opt/homebrew/opt/llvm@19/bin/clang++ make debug -j 8
LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config CXX=/opt/homebrew/opt/llvm@19/bin/clang++ ./scripts/test_all.sh
```

Results:

- `git diff --check`: passed.
- `python3 scripts/generate_instantiations.py`: regenerated 125
  instantiations.
- `make debug -j 8`: passed with existing LLVM/header warnings.
- `scripts/test_all.sh`: exited 0 and printed `All checks passed`.
  The existing `intense-case.cc` environment warning
  `fatal error: 'iostream' file not found` is still emitted before the script
  continues to its summary.

Targeted SQLite checks:

```bash
sqlite3 tests/output/moderate-case.db ".tables" | tr ' ' '\n' | grep -E "template_template|usertypes"
sqlite3 tests/output/moderate-case.db "select count(*) from template_template_argument;"
sqlite3 tests/output/moderate-case.db "select count(*) from template_template_instantiation;"
sqlite3 tests/output/moderate-case.db "select id,name,kind from usertypes where kind=8;"
```

Observed outputs:

- Tables include `template_template_argument`,
  `template_template_instantiation`, and `usertypes`.
- `template_template_argument`: 2 rows.
- `template_template_instantiation`: 2 rows.
- `usertypes where kind = 8`: `TT` and `C`.
- `template_template_argument_value`: not created or written in this phase.
