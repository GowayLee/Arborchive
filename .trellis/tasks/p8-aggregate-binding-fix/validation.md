# P8 aggregate binding and identity repair validation

Base: upstream/main `4e08b60273b9784e951a0cbbea0ccc51d5d67175`.

## Two distinct audited causes

1. Match semantic initializer slots rather than field declaration ordinals:
   direct bases consume leading slots, unnamed bitfields consume none, and a
   union binds only its active member. Field `position` stays the semantic
   initializer index; base slots can leave gaps in the field-only relation.
2. A brace-elided InitListExpr can share its full range with its child. Narrowly
   distinguish those aggregates by canonical type so nested lists and literals
   do not collapse into one ID. Ordinary expression and explicit-brace keys
   remain unchanged. Normalize syntactic lists to their semantic form.

Both changes stay within ExprProcessor and the existing expression-key helper.
ASTVisitor, schema and ORM are unchanged. Filtering self edges alone would not
restore the missing nodes or literal values, so the tests check those as well.

## Commands and results

```bash
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config \
  TOML_CFLAGS=-I/opt/homebrew/opt/toml11/include debug -j4
build/demo -c config.example.toml \
  -s tests/unit-tests/p8/aggregate_binding_case.cc -o /tmp/p8-aggregate.db
python3 scripts/assert_p8_aggregate_db.py /tmp/p8-aggregate.db --mutation-tests
JOBS=4 bash scripts/test_all.sh
```

| Gate | Observed result |
|---|---|
| Baseline + new checker | extraction 0; checker 1; misbound fields, lost values and self edges |
| Modified exact contract | 10 cases, 24 exact field/value/position bindings, 4 exact base-parent edges; exit 0 |
| Corrupt-copy controls | `P8_MUTATIONS_OK cases=9 source_unchanged=yes`; exit 0 |
| Full suite | 32 fixtures PASS; 10 existing DB warnings; exit 0 |
| Separate-copy rollback | both production hashes restored; rebuild/extract 0; checker 1 again |

Negative controls include wrong field/value/position, missing and extra facts,
self initializer, corrupted literal, missing field parents and separately deleting
all four base-parent edges. The last control closes a blind spot found during
independent review. The fixed branch remains modified after rollback testing.

Independent additional probes passed for nested arrays, array members, aliases /
const, nested designated unions, multiple elided bases, anonymous union and empty
base forms. They are supplemental observations, not a universal C++ guarantee.

## Limits

ImplicitValueInitExpr for omitted members remains an existing unsupported source
family: partial/empty records can still have initializer=-1. Both baseline and
modified probes confirm that debt; this patch does not claim complete aggregate
initialization support. Constructor/default-member initializers and other
previously deferred families are not expanded. Old DBs must be regenerated;
there is no schema migration. The 10 existing DB warnings remain visible.

## Follow-up review correction

The display-name suffix above was insufficient for shadowed record declarations.
See [revision-f04.md](revision-f04.md) for canonical USR plus physical declaration
identity, exact shadow/template regressions, 34-fixture full-suite verification,
separate-copy rollback and the subsequent independent review. Its explicit-brace
generic-template-root and source-less-type limits remain documented separately.
