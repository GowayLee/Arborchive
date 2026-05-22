# P4 Namespace / Using / Ownership Model

P4 subsystem architecture documentation. Describes the semantic model,
implementation invariants, and intentionally deferred areas.

---

## Semantic Entity vs Declaration Layering

P4 follows the CodeQL two-layer model:

```
Semantic entity layer         Declaration layer
─────────────────────         ─────────────────
namespaces                    namespace_decls
  (canonical identity,          (per-occurrence, fresh ID,
   shared across reopened       points to semantic entity via
   namespace blocks)            namespace_id)
```

A namespace that appears in multiple translation units (or is reopened in the
same TU) has **one** `namespaces` row and **N** `namespace_decls` rows.

This layering is the same pattern used by:
- P1 frienddecls: `declarations` (semantic) → `frienddecls` (declaration)
- P2 template system: template entities → template parameter/argument rows
- Future P5: `usertypes` (semantic) → `type_decls` (declaration)

---

## Canonical Namespace Identity

### Rule

Namespace identity is determined by `getCanonicalDecl()`, not by name or
source location.

### Implementation

```cpp
// namespace_processor.cc:45-67
int getOrCreateNamespaceId(const clang::NamespaceDecl *decl) {
  const clang::NamespaceDecl *canonical_decl = decl->getCanonicalDecl();
  // cache hit → return existing ID
  // cache miss → GENID(Namespace), insert, cache
}
```

### Invariants

- Reopened namespace blocks share the same `namespace_id`
- Anonymous namespaces get their own canonical identity (Clang assigns
  unique anonymous namespace per TU)
- Inline namespaces are separate semantic entities (recorded in
  `namespace_inline`)

### Verification

```sql
-- Reopened namespace should have 1 namespace row, N declaration rows
SELECT ns.name, COUNT(DISTINCT nd.id) AS decl_count
FROM namespaces ns
JOIN namespace_decls nd ON nd.namespace_id = ns.id
WHERE ns.name = 'p4_reopened'
GROUP BY ns.id;
-- Expect: decl_count >= 2
```

---

## namespace_decls Semantics

### Table

```
namespace_decls(id, namespace_id, location, bodylocation)
```

### Invariants

- `id`: always fresh (`GENID(NamespaceDecl)`)
- `namespace_id`: references `namespaces.id` (canonical semantic entity)
- `location`: declaration location (start of namespace block)
- `bodylocation`: body location (right-brace, falling back to end-loc)

### Edge case: empty namespace

```cpp
namespace p4_decl_reopened {}
namespace p4_decl_reopened {}
```

Produces: 1 `namespaces` row, 2 `namespace_decls` rows. Both declarations
share the same canonical identity but have separate locations.

---

## usings Semantics

### Table

```
usings(id, element_id, location, kind)
```

### Kind values (per dbscheme)

| kind | meaning |
|------|---------|
| 1 | `@using_declaration` (`using X::Y`) |
| 2 | `@using_directive` (`using namespace X`) |
| 3 | `@using_enum_declaration` (`using enum X`) — **deferred** |

### Current safe subset

- `UsingDecl` → kind=1
- `UsingDirectiveDecl` → kind=2
- `UnresolvedUsingTypenameDecl` → kind=1 (treated as using declaration)

### Invariants

- `id`: always fresh (`GENID(Using)`)
- `element_id`: **currently always -1** (target binding deferred)
- `location`: declaration location
- Implicit using declarations are skipped (`decl->isImplicit()` guard)

### Deferred: element_id (target binding)

`element_id` should reference the declaration being brought into scope.
Resolution requires:
- `UsingShadowDecl` traversal (Clang generates implicit shadow declarations)
- Name lookup in the target scope
- Template-dependent using handling (`UnresolvedUsingTypenameDecl`)

This is deferred because:
- Shadow declaration graph requires visibility/scope infrastructure
- Template-dependent usings require instantiation context
- The column exists in schema and is correctly typed; only the value is
  pending

---

## using_container Semantics

### Table

```
using_container(parent, child)
```

### Semantics

Expresses **lexical ownership only**: which scope contains this using
declaration/directive.

`parent` is the owning element (namespace or file).
`child` is the `usings.id`.

This is NOT a semantic visibility graph. It answers "where is this using
written?" not "what does this using make visible?"

### Supported owner contexts

| DeclContext type | Resolution | ID source |
|-----------------|------------|-----------|
| `NamespaceDecl` | `getOrCreateNamespaceId()` | `namespaces.id` |
| `TranslationUnitDecl` | `CompRecorder::getSourceFileId()` | `files.id` |
| `CXXRecordDecl` (class) | **skipped** | — |
| `FunctionDecl` | **skipped** | — |
| `BlockDecl` | **skipped** | — |

### Invariant: unsupported contexts MUST NOT emit fake edges

When `resolveUsingOwnerElement()` returns `nullopt`, the `using_container`
row is silently skipped. No `-1` foreign key is inserted.

```cpp
// namespace_processor.cc:137-143
if (auto parent_id = resolveUsingOwnerElement(decl->getDeclContext())) {
  // emit using_container row
} else {
  LOG_DEBUG << "Skipped using_container for unsupported owner context";
}
```

### Deferred owner contexts

Class-level and function-level ownership requires:
- P5 class hierarchy (for `CXXRecordDecl` as owner)
- Function/block scope model (for `FunctionDecl` / `BlockDecl` as owner)
- Universal DeclContext-to-ID resolution infrastructure

---

## Deferred Semantics Registry

### D1: UsingShadowDecl Graph

- **Why deferred:** Shadow declaration traversal requires visibility/scope
  infrastructure that doesn't exist yet. It's an incremental addition on top
  of the current using layer.
- **Architecture risk:** Low. Can be added without schema changes.
- **Prerequisites:** Name lookup infrastructure, DeclContext-as-owner support.

### D2: Target Binding (usings.element_id)

- **Why deferred:** Needs UsingShadowDecl + name lookup + template-dependent
  handling. The column exists with correct type; only the value resolution
  is pending.
- **Architecture risk:** Medium. Limits graph query capability from `usings`
  to target declarations.
- **Prerequisites:** D1 (UsingShadowDecl).

### D3: Class/Function/Block Ownership

- **Why deferred:** Requires P5 class hierarchy and function/block scope
  models. These are separate roadmap phases.
- **Architecture risk:** Low. Current implementation correctly skips
  unsupported contexts without fabricating data.
- **Prerequisites:** P5a (class hierarchy), function scope model.

### D4: Universal DeclContext Resolution

- **Why deferred:** A general DeclContext→element_id resolver that works
  for all context types is a cross-phase infrastructure item.
- **Architecture risk:** Low for P4. `resolveUsingOwnerElement()` is
  designed to accept new context types incrementally.
- **Prerequisites:** P5, P6 (lambda), function scope model.

### D5: Semantic Visibility Graph

- **Why deferred:** Full C++ name visibility is a query-layer concern that
  builds on top of all extracted tables. It requires namespace hierarchy,
  using relationships, class inheritance, and scope chains.
- **Architecture risk:** None for extraction. This is a consumer-side
  concern.
- **Prerequisites:** All declaration-layer tables + class hierarchy.

### D6: Implicit Generated Using Handling

- **Why deferred:** Implicit using declarations are compiler-generated
  artifacts. The extraction policy (which ones to record, and with what
  semantics) needs design before implementation.
- **Architecture risk:** Low. Current policy (skip all implicit) is
  consistent and explicit.
- **Prerequisites:** Design decision on implicit using policy.

### D7: @using_enum_declaration (kind=3)

- **Why deferred:** C++20 `using enum` has low frequency in target
  codebases. Pure incremental addition.
- **Architecture risk:** Low. Requires only new Visit* + processor method.
- **Prerequisites:** None (standalone addition).

### D8: Namespace Fully Qualified Names

- **Why deferred:** Design choice. The schema relies on `namespacembrs`
  graph for hierarchy; FQN is a query-layer derivation.
- **Architecture risk:** None. `namespaces.name` stores the local name
  intentionally.
- **Prerequisites:** None (query-layer concern).

---

## Extractor Invariants

1. **Canonical identity:** namespaces use `getCanonicalDecl()` — reopened
   blocks share the same semantic ID.

2. **Declaration freshness:** `namespace_decls` always use fresh
   `GENID(NamespaceDecl)` — each declaration occurrence gets its own row.

3. **Using layer:** `usings` are declaration-layer entities. Target binding
   is deferred but the schema column exists.

4. **Lexical ownership only:** `using_container` expresses where a using is
   written, not what it makes visible.

5. **No fake edges:** unsupported ownership contexts must NOT emit
   `using_container` rows with placeholder IDs.

6. **ASTVisitor dispatch-only:** All P4 semantic logic lives in
   `NamespaceProcessor`. The 4 Visit* methods are single-line delegations.

7. **Implicit skipping is explicit:** `recordUsing()` has a clear
   `isImplicit()` guard with a log message. The policy is transparent.

---

## Architecture Health

### Processor boundaries

`NamespaceProcessor` has one responsibility: namespace/using extraction.
It does not:
- Directly access other processors' internal state
- Manage cache policy (only an internal `unordered_map` for canonical decls)
- Orchestrate cross-table coordination
- Touch `DependencyManager` or `CacheManager`

### ASTVisitor compliance

All 4 P4 Visit* methods are 4-line dispatch-only. This is the cleanest
subsystem boundary in the current Arborchive codebase.

### Future expansion risks

- Adding `VisitUsingShadowDecl` / `VisitUsingEnumDecl` → pure incremental
  (new 4-line Visit* + processor method), no architectural risk
- Adding class/function ownership → requires coordinator or helper for
  DeclContext-to-ID resolution; the `resolveUsingOwnerElement()` method is
  designed for incremental context type addition
- Universal DeclContext resolver → risk of over-engineering; prefer
  incremental context type additions over a monolithic resolver

---

## Verification

```bash
# Full test suite (currently only slight/moderate/intense cases)
scripts/test_all.sh

# Manual namespace DB verification
./build/demo -c config.example.toml -s tests/unit-tests/namespace.cc -o /tmp/ns.db
scripts/db_summary.py /tmp/ns.db
sqlite3 /tmp/ns.db 'SELECT COUNT(*) FROM namespaces;'
sqlite3 /tmp/ns.db 'SELECT COUNT(*) FROM namespace_decls;'
sqlite3 /tmp/ns.db 'SELECT COUNT(*) FROM usings;'
sqlite3 /tmp/ns.db 'SELECT COUNT(*) FROM using_container;'
sqlite3 /tmp/ns.db 'SELECT COUNT(*) FROM namespacembrs;'
```

## Related Documents

- AGENTS.md — architecture governance
- docs/AGENT_WORKFLOW.md — execution workflow
- docs/roadmap.md — phase planning
- docs/semmlecode.cpp.dbscheme — canonical schema reference
- docs/datatable-list.txt — table inventory
