# P1 friend declaration semantics repair

## Goal and ownership

Correct `frienddecls.type_id` to the declaring/befriending class and `decl_id`
to the friend function or class. The schema is unchanged. `TemplateProcessor`
owns resolution and persistence; `VisitFriendDecl` remains dispatch-only.

The CodeQL consumer is `cpp/ql/lib/semmle/code/cpp/FriendDecl.qll`:
`getDeclaringClass()` consumes column 2 and `getFriend()` column 3.

## Scope

- Resolve the owner through `FriendDecl::getDeclContext()` and TypeProcessor.
- Desugar non-dependent class aliases; preserve forward class references.
- Resolve function targets through the existing canonical-key dependency path,
  without eagerly reprocessing function declarations or writing invalid rows.
- Keep distinct source occurrences and different owners distinct.
- Skip ignored non-class type friends, dependent type targets and unsupported
  template-declaration targets rather than manufacturing invalid declaration IDs. A dependent owner alone
  is not a reason to discard a concrete class target.

## Allowed paths / non-goals

Allowed: `src/core/processor/template_processor.cc`, its header only if needed,
`tests/unit-tests/p1/`, `scripts/assert_p1_friend_db.py`, the minimal test runner
hook, and this task's plan/validation notes.

Do not change ASTVisitor, schema/model/ORM, other processors, P8/P11, or broaden
the template semantics roadmap. Existing unrelated test warnings remain visible.

## Verification

1. Extract a new fixture with the unchanged baseline; require exact semantic
   assertions to fail (not just a successful extraction or row count).
2. Apply the processor repair and check owner/target kind/name/identity, source
   occurrence, reference validity, forward/hidden/overloaded/member friends,
   aliases, nested owners and supported concrete targets in template owners.
3. Damage read-only-backup database copies; require every mutation to fail and
   prove the source DB hash is unchanged.
4. Run `bash scripts/test_all.sh` in the isolated branch and inspect DB output.
5. Reverse the patch on another copy, verify baseline source hashes and rerun
   the new semantic test against the restored behavior.

## Risks

Function and type identity have pre-existing behavior outside P1. Do not refactor
their global key/cache policy here. Dependency callbacks must not enqueue more
dependencies, retain mutable processor state, or insert unresolved placeholders.
Old databases need regeneration to replace incorrect facts; no schema migration
is needed.

## Review correction: alias target metadata

Keep the record-type semantic processing when resolving desugared class targets;
TypeProcessor still owns completeness/POD/standard-layout facts. No visitor or
TypeProcessor implementation change is required. Add independent system-header
alias/direct targets for POD, non-POD/non-standard-layout and incomplete classes.
Check target identity, exact source coordinates and positive/negative trait facts;
mutate copies to demonstrate that missing, wrong-owner and invented facts fail.
Run the pre-correction binary against the new fixture, then build/full suite and
independent review. Reverse only this correction in a disposable source/build copy
and require the same trait regression to reappear, leaving the fixed branch intact.
