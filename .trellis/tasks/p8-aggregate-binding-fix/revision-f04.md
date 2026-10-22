# PR #19 F04 identity follow-up

Previous PR head: `37c399e6275a273d69d4ee462b54f63cfd7f80a8`.

## Scope and ownership

The review's global/local same-name record example still collapsed two
brace-elided InitListExpr nodes after the first repair. A canonical type's
printed name is not declaration identity. This follow-up changes only the
existing same-source-range suffix in `src/util/key_generator/expr.cc`.
ASTVisitor, TypeKey, schema, ordinary expressions, explicit-brace roots, and
other roadmap phases are unchanged.

## Identity rule

- Retain canonical type shape and append the Clang canonical type USR.
- For a record or array of records, also append its canonical RecordDecl's
  physical spelling and expansion filename plus file offset.
- USR distinguishes template contexts and includes array element/extent data.
  Instantiated function-local shadowed records can nevertheless share a USR;
  canonical declaration source identity distinguishes those records.
- Physical offsets are deliberate: `#line` can make two different declarations
  display the same filename, line and column.
- If USR generation reports failure, discard its partial buffer and retain
  canonical shape plus the valid record declaration source identity. A
  source-less unsupported type still has the prior canonical-shape limitation;
  this patch neither claims universal identity nor adds a global throw/pointer
  fallback. All exercised concrete record/array types generated USRs successfully.

## Formal regressions

`aggregate_identity_case.cc` and `assert_p8_identity_db.py` check four complete
initialization graphs: global/local shadowing, nested-block shadowing,
record-array shadowing, and alias/forward-declaration shadowing. Declarations
are resolved through source markers rather than type display names. The checker
asserts root identity, braced roots, 14 distinct aggregate nodes, 12 exact field
bindings, two array bindings, reachable literal values and the complete parent
edge multiset. Thirteen corrupt-copy controls cover collapsed identity, missing
root/braced facts, disconnected leaves, extra edges, bad fields/positions and
missing nodes; source DB hashes remain unchanged.

`aggregate_template_identity_case.cc` and its dedicated checker cover one
function-template instantiation with two same-name inner records and `#line`
remapping both declarations to the same displayed line and column. Its exact
contract covers two inner aggregate nodes, two field bindings, a reachable
literal and both parent edges. Six corrupt-copy controls must fail.

The generic explicit-brace root has an existing source-key limitation: its
syntactic/template form can occupy the instantiated root's key. The focused
template checker does not claim to repair that root or full multi-instantiation
initialization graphs. It does not suppress failures in the four complete-graph
cases. A separate production-key probe checks partial/full class-template
contexts and int/long function-template contexts without broadening the patch.

## Targeted validation

Saved pre-revision binary: `.git/pr18-pr20-revision-backups/p8/demo` in the main
checkout. Evidence: `docs/analysis/pr18-pr20-revisions-2026-10-22/p8/` in that
checkout.

- Pre-revision extraction: exit 0 for both new fixtures; both new checkers exit 1.
- LLVM19 `make ... debug -j4`: exit 0; existing LLVM/header/platform warnings.
- Four complete-graph cases: exit 0; 13 mutations rejected.
- Focused template + `#line`: exit 0; six mutations rejected.
- Existing aggregate fixture: 10 cases, 24 bindings, four base edges and nine
  mutations still pass.
- Common DB checks for both new fixtures: exit 0, zero warnings.
- Production-key probe: normal same-range nodes and all tested template
  contexts are distinct; repeated independent extraction gives identical keys.

The main thread owns the subsequent full suite, separate-copy rollback,
independent review, commit and push. Their final evidence is recorded separately;
this note reports only the targeted checks performed during implementation.

## Final verification and independent review

- `JOBS=4 bash scripts/test_all.sh`: 34 fixtures PASS, 10 existing DB warnings,
  exit 0, including both new regression checkers and their mutation controls.
- Another source/build copy reversed only this follow-up production patch;
  the pre-revision source SHA was restored, rebuild/extraction exited 0, and
  all four complete-graph regressions failed again with checker exit 1.
  The working branch remained corrected.
- Independent reviewer found no blocking issue in the bounded correction:
  13 external complete-tree probes passed; ten failed with the old binary,
  three controls passed on both. The intermediate USR/presumed-location #line
  collision was independently reproduced and verified absent with physical
  offsets. int/long instantiations retain four distinct inner aggregates and
  two complete inner chains; their old explicit-brace-root limitation is not
  described as repaired.
- The reviewer freshly extracted all three official P8 fixtures: 13 new graph,
  six template-subgraph and nine existing corrupt-copy controls passed.
