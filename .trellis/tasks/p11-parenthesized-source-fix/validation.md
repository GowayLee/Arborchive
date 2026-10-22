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

## Review-driven revision: bound methods are not field-read sources

The review's valid `(box.method)()` counterexample added a fake field-read and
attached the same receiver to two main-tree parents. Conversion-source lookup
now excludes non-static CXXMethodDecl before calling the existing field-read
processor. FieldDecl sources, including fields holding function pointers, keep
their original conversion/load paths. Static member and enum semantics are not
expanded by this revision.

Targeted commands (same build flags as above):

```bash
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config \
  TOML_CFLAGS=-I/opt/homebrew/opt/toml11/include debug -j4
build/demo -c config.example.toml \
  -s tests/unit-tests/p11/parenthesized_sources_case.cc -o <revision-db>
python3 scripts/assert_p11_parenthesized.py <revision-db> --mutations
```

| Revision gate | Observed result |
|---|---|
| Expanded fixture syntax (Clang 19 C++20) | exit 0 |
| Saved pre-revision binary + revised checker | extraction exit 0; checker exit 1, 9 errors for the four parenthesized method cases |
| Revised exact source contracts | `[p11_parenthesized] PASS (23 source cases)`; exit 0 |
| Revised corrupt-copy controls | `[p11_mutation] PASS (28 corruptions rejected; source unchanged)`; exit 0 |

The 23 cases comprise the existing 14 sources, six dot/arrow method-call forms
(plain, single and nested parentheses), and three function-pointer field reads.
The checker now enforces one main parent, exact scalar/receiver parameter
owner/index/name, field read kind and exact base edge. Same-name parameter
misbindings across functions are explicitly rejected. The 28 corruption cases
must each change at least one row before rejection counts as evidence.

The following integration-task gates were then completed on the same frozen
source and tests:

- `JOBS=4 bash scripts/test_all.sh`: 32 fixtures PASS, 10 existing DB warnings,
  exit 0. No new schema or Visitor changes were introduced.
- A separately marked copy reversed only this revision's production patch,
  restored SHA-256
  `841592418fb14ea3051b5f3f593583fbb54a7ab095aba7162f3159e418466c1a`,
  and rebuilt successfully. Extraction returned 0; the same new regression
  returned 1 with the same 9 errors. The actual PR branch retained the fix.
- An independent reviewer replayed the original minimal finding (two receiver
  parents / one false field-read became one / zero), validated 24 external
  cases, and confirmed unchanged semantic fingerprints for eight positive
  controls, including actual dot/arrow callback calls and callable-object
  fields. The reviewer independently reran the 23-case/28-mutation contract
  and rejected six further corrupt-copy controls. No new blocker was found
  within that review's stated scope.

Local command/input/literal-output/exit-status evidence and the independent
review are archived under
`docs/analysis/pr18-pr20-revisions-2026-10-22/` in the audit checkout.
The existing routineptr underlying `type_id=-1` and duplicate routine type IDs
remain unchanged and are not claimed repaired.
