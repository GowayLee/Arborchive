# AGENTS.md

Arborchive architecture governance for AI coding agents and maintainers.

This document defines the architectural boundaries of the project.
It is not a general development note or onboarding tutorial.

---

# Documentation Hierarchy

Priority order:

text AGENTS.md ↓ docs/AGENT_WORKFLOW.md ↓ docs/roadmap.md ↓ existing code patterns

Canonical workflow file: `docs/AGENT_WORKFLOW.md`.
Legacy/onboarding encyclopedia: `docs/AGENTS_GUIDE.md`.

If existing code conflicts with this document:

text AGENTS.md wins.

If `docs/AGENTS_GUIDE.md` conflicts with this document, `AGENTS.md` wins.

Historical code patterns are not architectural truth. They may document current
or legacy implementation facts, not approved architecture.

---

# Core Principles

- 默认使用中文回复。
- 小步 patch：保持可回滚、可验证、可 review。
- 以 semantic subsystem 为单位推进功能，而不是随机补表。
- 优先保持 architecture boundary，而不是追求局部快速实现。
- 不做无关大型重构，除非用户明确要求。

Every patch should:
- explain the plan before modification
- report changed files
- report validation commands/results
- report remaining risks

---

# Architectural Boundaries

## ASTVisitor Target Boundary

Architectural target: `src/core/ast_visitor.cc` should remain as close as
possible to a dispatch-only layer.

`Visit*` methods should normally only:
- identify AST entrypoints
- perform lightweight guards
- delegate work to processors/helpers

The current codebase is transitional. Some legacy orchestration still exists
inside `ASTVisitor`. These patterns are technical debt, not preferred
architecture and not a template for new work.

## Transitional Legacy Patterns

Tolerated temporarily when already present:

- lightweight multi-processor delegation needed for compatibility during subsystem migration
- narrow compatibility glue while moving ownership into processors/helpers
- small guard logic required to preserve traversal behavior

These patterns MUST NOT expand further. New orchestration MUST NOT be added to
`ASTVisitor`. When touching legacy orchestration-heavy code, prefer shrinking
the visitor boundary or moving logic into processor/helper layers.

## Visit* Examples

Preferred pattern:

```cpp
bool ASTVisitor::VisitFooDecl(clang::FooDecl *D) {
  if (!D) return true;
  foo_processor_->processFooDecl(D);
  return true;
}
```

Transitional legacy pattern, tolerated only when already present:

```cpp
bool ASTVisitor::VisitFooDecl(clang::FooDecl *D) {
  if (!D) return true;
  type_processor_->processType(D->getType());
  foo_processor_->processFooDecl(D);
  return true;
}
```

This is limited orchestration for compatibility. It is not preferred for new
work and should not grow.

Forbidden pattern:

```cpp
bool ASTVisitor::VisitFooDecl(clang::FooDecl *D) {
  auto *Special = llvm::dyn_cast<clang::SpecialFooDecl>(D);
  auto key = KeyGenerator::makeFooKey(D);
  if (CacheManager::instance().find(key)) return true;
  DependencyManager::instance().addDependency(...);
  STG.insert(...);
  // many schema-specific branches and table coordination
  return true;
}
```

Forbidden inside visitor code:

- direct `STG.insert*`
- direct `DependencyManager` orchestration
- direct `CacheManager` orchestration
- complex `dyn_cast` routing trees
- schema row assembly
- subsystem controller logic

## Architecture Drift Signals

The following signals should trigger ownership review:

- new `Visit*` methods call multiple processors
- `Visit*` methods directly use `STG`, `DependencyManager`, or `CacheManager`
- visitor code adds cache/key/dependency policy
- repeated branching grows in visitor methods
- existing orchestration-heavy `Visit*` methods receive more orchestration
- a patch cannot be verified as an isolated semantic subsystem change

Existing orchestration-heavy `Visit*` methods are legacy technical debt. Do not
expand orchestration inside them.

# Historical Failure

Arborchive previously suffered architecture degradation because template orchestration and schema coordination logic gradually expanded inside:

text src/core/ast_visitor.cc

This caused:
- visitor bloating
- subsystem coupling
- difficult rollback
- difficult verification
- uncontrolled branching growth

The issue was later fixed through:
- TemplateProcessor orchestration
- visitor slimming
- extraction boundary refactoring

This is a known anti-pattern.

Future agents MUST NOT repeat it.

---

# Processor Ownership

Semantic ownership belongs to processors and helper layers.

Examples:

- template semantics → TemplateProcessor
- type extraction → TypeProcessor
- inheritance extraction → dedicated hierarchy/layout path
- attribute extraction → attribute subsystem
- lambda extraction → lambda subsystem
- initialization extraction → initialization subsystem
- expression graph extraction → expression subsystem

ASTVisitor does not own semantic pipelines.

If a subsystem has no clear owner:
- establish processor/helper boundaries first
- then implement extraction logic

## Coordinator Layer Guidance

Complex subsystem coordination may live in:

- dedicated coordinator/helper layers
- dominant processors coordinating internally within their subsystem

`ASTVisitor` must not become the coordination layer. This is a governance rule,
not an implementation plan; introduce concrete coordinators only when a real
subsystem boundary needs them.

---

# Refactor Triggers

Refactor boundaries before continuing implementation if:

- a new Visit* cannot stay dispatch-oriented
- repeated branching accumulates in visitor code
- multiple schema tables are coordinated inside visitor methods
- subsystem ownership becomes unclear
- semantic orchestration starts leaking into AST traversal
- rollback or isolated verification becomes difficult

Do not continue patching on top of architectural drift. In legacy transitional
areas, a small compatibility edit may be acceptable only if it does not expand
or normalize visitor orchestration.

---

# AI-Agent Rules

AI agents MUST:
- follow subsystem ownership
- keep ASTVisitor thin
- prefer processor/helper extraction
- avoid copying historical bad patterns
- explain architectural reasoning before changes

AI agents MUST NOT:
- use visitor code as temporary orchestration space
- expand bad local patterns because they already exist
- mix unrelated roadmap phases
- implement cross-table semantic pipelines directly in Visit*

---

# Required Context Before Changes

Before modifying C++ behavior, agents MUST understand:

- current roadmap phase
- AST dispatch flow
- related processor ownership
- related schema/model definitions

Minimum required files:

- AGENTS.md
- docs/AGENT_WORKFLOW.md
- docs/roadmap.md
- src/core/ast_visitor.cc
- related processor/model/table files

---

# Schema Constraints

- 不允许新增 docs/datatable-list.txt 之外的新表。
- 表名必须严格遵循 datatable-list。
- 字段语义优先参考 docs/semmlecode.cpp.dbscheme。
- schema patch 必须服务于明确 semantic subsystem。
- 禁止随机补表式开发。

If schema/ORM changes:
- update model/table_defs/table_init consistently
- regenerate ORM instantiations
- rerun full validation

---

# Roadmap Discipline

Roadmap phases MUST remain isolated.

Examples:

- namespace phases should not mix layout extraction
- inheritance graph should not mix ABI layout
- lambda phase should not absorb unrelated CFG work
- attribute presence and attribute argument systems should remain separated

Do not expand deferred tables into unrelated refactors.

---

# Minimal Validation

Minimum validation after patches:

bash scripts/test_all.sh

If schema/ORM changed:

bash python3 scripts/generate_instantiations.py make debug -j 8 scripts/test_all.sh

Use isolated testcases and DB checks when semantic output changes.

---

# Key Paths

- AST dispatch: src/core/ast_visitor.cc
- Processors: src/core/processor/
- DB models: include/model/db/
- Table definitions: include/db/table_defs/
- ORM/storage: include/db/, src/db/
