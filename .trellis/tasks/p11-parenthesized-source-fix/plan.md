# P11 parenthesized conversion-source repair

## Goal and ownership

Repair the audited loss of routine/member conversion sources under parentheses.
ExprProcessor owns source resolution, conversion chains and load placement.
ASTVisitor remains unchanged. Schema/table/model/ORM are unchanged.

## Allowed paths

- src/core/processor/expr_processor.cc: conversion-source and load-source lookup.
- tests/unit-tests/p11/parenthesized_sources_case.cc: isolated regression input.
- scripts/assert_p11_parenthesized.py: exact semantic and mutation assertions.
- scripts/test_all.sh: add the fixture and invoke its assertions.
- This task's plan and validation notes.

Other phases, the general MemberExpr subsystem, dependent expressions, temporary
initialization, public APIs, schema and unrelated visitor debt are not changed.

## Plan

1. Add one-line, source-marked fixtures for scalar controls, point/arrow member
   reads, routine decay, address-of controls and nested parentheses.
2. Verify exact source-to-wrapper chains, endpoint identities, type/category,
   generated/cast metadata, load ownership and main-tree wrapper exclusion.
3. Run the assertions against the unchanged binary and require failure.
4. Reuse conversion-source resolution in ParenExpr; keep load markers on the
   normalized underlying source rather than on a parenthesis wrapper.
5. Run the corrected fixture, deliberate DB corruption controls and full suite.
6. Review diff boundaries and retain rollback evidence on a separate copy.

## Risks

Do not materialize a general member-expression graph merely to fix conversion
sources. Recursion must descend through the AST and use existing expression
caches. Extra parentheses must preserve routine bindings and member load flags,
not merely add the expected number of conversion rows. Tests discover runtime
IDs from source locations; no database IDs are fixed in the assertions.

## Review-driven bound-method correction

The parenthesized-source repair also reached non-static CXXMethodDecl member
expressions. These are bound methods owned by the real call, not field reads;
materializing them added a second main-tree parent to the receiver.

- Guard only non-static CXXMethodDecl in conversion-source lookup, before the
  existing member-read path. Keep FieldDecl sources, including callback fields.
- Add point/arrow method calls with no, one, and two parentheses, plus callback
  field positive controls. Assert the exact real call, receiver binding and
  single receiver-parent edge rather than merely count conversion nodes.
- Strengthen scalar/field source identities through parameter owner/index/name,
  exact field read kinds and base edges. Reject same-name cross-function binds,
  missing base edges and additional receiver parents in corrupt-copy controls.
- Do not expand static member, enum, dependent or general member-call modeling;
  these remain outside this narrowly scoped correction.
