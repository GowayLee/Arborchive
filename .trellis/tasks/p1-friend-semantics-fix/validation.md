# P1 repair validation

Base: upstream/main `4e08b60273b9784e951a0cbbea0ccc51d5d67175`.
The base tree is identical to audited P13 HEAD `761aa51`.

## Change and evidence

`TemplateProcessor::processFriendDecl` now stores the befriending class in
`type_id` and the friend entity in `decl_id`, as consumed by CodeQL
[`FriendDecl.qll`](https://github.com/github/codeql/blob/main/cpp/ql/lib/semmle/code/cpp/FriendDecl.qll).
Function targets use the existing canonical-key dependency resolver without an
initial unresolved row. Class aliases and forward class targets resolve through
TypeProcessor. Different source occurrences remain distinct. Ignored non-class
friends and dependent type / unsupported template-declaration targets do not
produce invented endpoints.

ASTVisitor, schema, ORM and unrelated phases are unchanged.

## Commands and results

```bash
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config \
  TOML_CFLAGS=-I/opt/homebrew/opt/toml11/include debug -j4
build/demo -c config.example.toml \
  -s tests/unit-tests/p1/friend_semantics_case.cc -o /tmp/p1-friend.db
python3 scripts/assert_p1_friend_db.py /tmp/p1-friend.db --mutation-tests
JOBS=4 bash scripts/test_all.sh
```

| Gate | Observed result |
|---|---|
| Baseline binary + new exact checker | extraction 0; checker 1; owner/target identities wrong |
| Modified exact contract | `P1_FRIEND_OK relations=14 exact_owner_target=yes`; exit 0 |
| Corrupt-copy controls | `P1_MUTATIONS_OK cases=9 source_unchanged=yes`; exit 0 |
| Full suite | 32 fixtures PASS; 10 pre-existing DB warnings; exit 0 |
| Separate-copy rollback | reverse production patch; source hash restored; rebuild 0; extraction 0; checker 1 again |

Rollback was tested on a separate copy, not merely a reverse-patch dry run.
The fixed branch was left unchanged. The original source SHA-256 is
`5c9e2e940c9d362f3e28b6de2646b468e45ec0398a2f8ccc97007c9c3d6c4172`;
local transaction records retain both actual SHA-256 values and full commands.

Independent probes also passed for constructor/destructor/conversion/non-static
member friends, same-named classes in different namespaces, and ignored enum /
non-class typedef friends. The committed fixture covers 14 exact relationships,
including forward, hidden, repeated, inline, overloaded, static-member, alias,
nested-owner and concrete-target-in-template-owner cases.

## Limits

Existing type/function identity behavior is not globally refactored. This is not
complete dependent/template friend support. Regenerate old databases to replace
incorrect facts; there is no schema migration. Compiler/linker warnings and the
10 existing DB warnings remain visible rather than suppressed.

## Review correction: preserve system-header alias target traits

A later review found that the direct declaration-ID lookup dropped record traits
for alias-only targets defined in skipped system-header subtrees. The correction
restores `TypeProcessor::processRecordType` on the desugared record path before
looking up the stable target ID. No visitor, schema or TypeProcessor edit.

The existing fixture now has 20 exact friend relationships and six independent
system-header alias/direct targets. POD, virtual non-POD/non-standard-layout and
forward-only classes have exact positive/negative completeness/POD/layout facts.
Owner/target kinds and full source ranges are now checked as well.

| Correction gate | Observed result |
|---|---|
| Pre-correction PR binary + expanded fixture | extraction 0; checker 1; alias POD loses 3 traits and alias non-POD loses completeness |
| Corrected binary | `P1_FRIEND_OK relations=20 exact_owner_target=yes kinds=yes source_ranges=yes trait_targets=6`; exit 0 |
| Mutation tests | `P1_MUTATIONS_OK cases=25 source_unchanged=yes`; exit 0 |
| Full suite | 32 fixtures PASS; 10 existing DB warnings; exit 0 |
| Isolated rollback | restore pre-correction source SHA, rebuild/extract 0; same 4 trait errors and checker exit 1 |

The baseline/direct-class/forward controls use distinct target declarations, so a
successful direct type visit cannot mask the alias-only regression. Expanded
negative controls reject missing, misbound and fabricated traits as well as wrong
entity kinds and source columns. The working branch stays corrected after rollback.

Independent post-correction review found no blocking issue in the bounded scope.
It freshly re-extracted the official fixture and checked 15 additional cases:
using/typedef chains, cv alias, decltype, alias templates, forward-then-defined,
non-POD but standard-layout, virtual, union and repeated aliases. All corrected
contracts passed; 11 alias regression cases failed with the pre-correction binary,
while four direct/incomplete controls stayed correct on both versions.
