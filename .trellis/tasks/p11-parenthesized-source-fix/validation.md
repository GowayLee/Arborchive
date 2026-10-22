# P11 parenthesized-source repair validation

Base: upstream/main `4e08b60273b9784e951a0cbbea0ccc51d5d67175`.

## Change

ParenExpr uses the same narrow conversion-source lookup as casts, preserving
routine-reference and member-read endpoints under one or more parentheses.
Lvalue-to-rvalue loads are recorded on the normalized underlying source rather
than on parenthesis wrappers. No ASTVisitor, schema, ORM or TypeProcessor change.

## Verification

```bash
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config \
  TOML_CFLAGS=-I/opt/homebrew/opt/toml11/include debug -j4
build/demo -c config.example.toml \
  -s tests/unit-tests/p11/parenthesized_sources_case.cc -o /tmp/p11-sources.db
python3 scripts/assert_p11_parenthesized.py /tmp/p11-sources.db --mutations
JOBS=4 bash scripts/test_all.sh
```

| Gate | Observed result |
|---|---|
| Baseline + corrected checker | extraction exit 0; checker exit 1 (6 semantic failures) |
| Modified exact chains | `[p11_parenthesized] PASS (14 source cases)`; exit 0 |
| Corrupt-copy controls | `[p11_mutation] PASS (13 corruptions rejected; source unchanged)`; exit 0 |
| Full suite | 32 PASS; 10 existing DB warnings; exit 0 |
| Separate-copy source rollback and rebuild | source hash restored; build/extract exit 0; checker exit 1 again |

The fixture covers scalar/member/arrow reads, ordinary and nested parentheses,
routine decay and address-of controls. It checks exact directed chains, type /
value category, routine binding, loads, generated/cast metadata, and exclusion
of conversion wrappers from the main expression tree. Rollback was executed on
a guarded separate copy, not only checked with a patch dry run; the fixed branch
was preserved.

## Test-model correction and limits

Independent review found that the original draft checker incorrectly expected
routine pointer kind 1 and identical IDs for structurally identical routine
types. The local dbscheme and DerivedTypeKind define routineptr as kind 6;
TypeProcessor currently allocates separate routine type IDs. Only routine-source
and parenthesis nodes therefore compare the exact `int(int)` signature,
parameter indices, qualifiers and category. Other types still require identity
agreement. Wrong routine return/argument and decay-kind corruptions are rejected.
The corrected checker still fails on the unchanged binary; this was not a
relaxation to accept missing conversion facts.

Baseline and modified output both retain a pre-existing routineptr underlying
`type_id=-1` in derivedtypes. Global routine-type identity/closure is not repaired
here. Full member-call modeling, dependent casts, temporary initialization and
other previously deferred expression families are not claimed. Regenerate old
DBs for repaired conversion facts; no schema migration is required.
