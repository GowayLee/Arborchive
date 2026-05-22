# Arborchive Roadmap

本文件记录 Arborchive 当前阶段性进展和后续语义抽取路线。详细 schema 以
`docs/datatable-list.txt` 为准；字段语义优先参考
`docs/semmlecode.cpp.dbscheme`。

## 工作边界

- 小步 patch，保持每一步可回滚、可验证。
- 不新增 `docs/datatable-list.txt` 之外的表名。
- 不用 `friend_decls`、`template_decls`、`template_parameters` 这类自定义表名替代目标表。
- 修改 schema / ORM 时同步更新 model、table_defs、table_init，并运行 `python3 scripts/generate_instantiations.py`。
- 每次功能 patch 后优先运行 `scripts/test_all.sh`，并检查生成的 SQLite 数据库。

## Roadmap Philosophy

P1-P3 已完成 Template / Concept Closure。项目路线从零散 schema patch 和
template orchestration cleanup，转向语义子系统补全和稳定的增量抽取阶段。

后续阶段按 compiler semantic subsystem 组织，而不是按随机剩余表清单推进。每个
phase 应只覆盖一个清晰的 AST/语义边界，优先做到：

- 子系统驱动：围绕 namespace、class hierarchy、lambda、attribute、initialization 等语义域推进。
- 小步 patch：每次只引入可解释、可验证、可回滚的 extractor 增量。
- 易验证：为目标表准备最小 AST testcase，并用 SQLite 摘要或 SQL 查询确认落库结果。
- 易回滚：避免跨 subsystem 的耦合改动，避免把 layout、attribute args、expression graph 等高风险内容混入低风险阶段。
- 隔离风险：dependent type、incomplete definition、ABI/layout、Expr/APValue serialization 等风险必须在对应 phase 内显式处理。

## Completed Phases

这些记录保留 P0-P3 的历史上下文，供后续贡献者 review 实现边界、deferred 决策和验证路径。

| Phase | Theme | Main tables | Commits | Status |
| --- | --- | --- | --- | --- |
| P0 DerivedTypes | record derived types for implicit casts | `derivedtypes`(81) | `a3ac8e6` | done / verified |
| P1 frienddecls | complete frienddecls 144 | `frienddecls`(144) | `5b07c13` | done / verified |
| P2 template system phase | template declaration markers, instantiations, arguments, value extraction safe subset, variable templates, template-template arguments, concept templates | 96-99, 102-109, 111, 113, 114, 116 | `9f395d6`, `f655a10`, `2e02b8a`, `b93ffa6`, `9770870`, `eb7035f`, `9a2c2fc`, `d6323a2` | stage complete |
| P3 template / concept closure | constraint expr extraction, type constraint binding, template-template semantics, non-type/value template argument support, deferred feasibility | 90, 91, 96-111 except 112, 113-117 | see P3 process notes | done / verified |
| P4 namespace / using / ownership | canonical namespace identity, namespace_decls extraction, using declarations/directives, lexical ownership (using_container) | `namespaces`(150), `namespace_inline`(151), `namespacembrs`(152), `namespace_decls`(68), `usings`(69), `using_container`(70) | `78e1a2b`, `7baaab1`, `9f9ec24` | stage complete |

### Template / Concept Closure

状态：P2/P3 已完成 template/concept 目标表实现收口，当前只保留一个有设计依据的 deferred。

DONE: 90, 91, 96-111 except 112, 113-117.

Deferred by design: 112 `template_template_argument_value`.

`template_template_argument_value.arg_value` is `@expr`, but Clang exposes
template-template arguments as `TemplateName` / `TemplateDecl`, not `Expr*`;
Arborchive records the supported relation through `template_template_argument`
and `template_template_instantiation`.

PR readiness: P2/P3 implemented 21 template/concept target tables; 112 remains
documented deferred.

### P3 Status

| Phase | Status | Notes |
| --- | --- | --- |
| P3a constraint expr extraction | done | Constraint expression support closed for the planned subset. |
| P3b type constraint binding | safe subset complete | Type constraint binding support closed for the planned subset. |
| P3c template-template instantiation semantics | safe subset complete | Process notes: `docs/my-docs/p3/p3c_template_template_semantics.md`. |
| P3d non-type/value template argument expr support | safe subset complete | Process notes: `docs/my-docs/p3/p3d_value_template_argument_support.md`; handoff: `docs/my-docs/p3/p3d_value_template_argument_handoff.md`. |
| P3e template-template argument value feasibility | feasibility complete | Process notes: `docs/my-docs/p3/p3e_template_template_argument_value_feasibility.md`; 112 stays deferred by design. |

## Completed Phases (continued)

### P4: Namespaces & Using System — stage complete

P4 完成了命名空间语义实体、声明层和词法所有权的基础设施。

Sub-phase breakdown:

#### P4a-0: Canonical Namespace Identity

- Status: DONE
- Key change: `getCanonicalDecl()` 去重，重开 namespace 块共享同一 semantic entity ID
- Tables affected: `namespaces`
- Commit: `7baaab1`

#### P4a-1: Namespace Declarations

- Status: DONE
- Tables: `namespace_decls`, `namespace_inline`, `namespacembrs`
- Key AST: `NamespaceDecl`
- Commit: `9f9ec24`

#### P4b-1: Using Declarations & Directives

- Status: DONE (safe subset)
- Tables: `usings`
- Key AST: `UsingDecl`, `UsingDirectiveDecl`, `UnresolvedUsingTypenameDecl`
- Note: `element_id` is intentionally deferred (see P4 ownership model)

#### P4b-2: Lexical Ownership

- Status: DONE (safe subset)
- Tables: `using_container`
- Scope: NamespaceDecl and TranslationUnitDecl owners only
- Note: class/function/block ownership intentionally deferred (see P4 ownership model)

Deferred items are documented in `docs/p4-ownership-model.md`.

### P5: Class Hierarchy & Record Layout

P5 拆分 inheritance graph 和 ABI layout extraction，避免把类型层级关系与 layout 风险混在同一个 patch 中。

#### P5a: Inheritance Graph

- Tables: `derivations`, `derspecifiers`
- Focus: inheritance graph、multiple inheritance、virtual inheritance relationships
- Key AST: `CXXRecordDecl`, `CXXBaseSpecifier`
- Complexity: Medium
- Boundary: 不在本阶段混入 layout extraction。

#### P5b: Record Layout & ABI Extraction

- Tables: `direct_base_offsets`, `virtual_base_offsets`, `fieldoffsets`
- Focus: `ASTRecordLayout` extraction、base offsets、virtual base offsets、field byte offsets
- Complexity: Medium-High
- Risk notes: dependent types 和 incomplete definitions 必须谨慎 guard，例如 `isDependentType()`、`hasDefinition()`。
- ABI notes: LLVM19 的 ABI/layout extraction 存在版本和目标平台差异风险，需要用最小 testcase 与 DB snapshot 固化期望。

### P6: Lambda System

- Tables: `lambdas`, `lambda_capture`
- Focus: `LambdaExpr`、capture models
- Complexity: Medium
- Boundary: 本阶段不包含 `code_block`。

### P7: Attribute System

P7 先抽取 attribute presence graph，再处理 argument system。presence 与 argument serialization 风险分离。

#### P7a: Attribute Presence Graph

- Tables: `attributes`, `typeattributes`, `funcattributes`, `varattributes`, `stmtattributes`
- Focus: GNU attributes、C++11 attributes、MS attributes
- Complexity: Medium

#### P7b: Attribute Argument System

- Tables: `attribute_args`, `attribute_arg_value`, `attribute_arg_type`, `attribute_arg_constant`, `attribute_arg_expr`, `attribute_arg_name`
- Focus: constant args、expr args、type args
- Complexity: High
- Risk notes: `Expr` / `APValue` serialization 复杂度高，应优先限定 safe subset，并保留无法稳定序列化的 deferred 记录。

### P8: Initialization System

- Tables: `initialisers`, `braced_initialisers`, `aggregate_field_init`, `aggregate_array_init`
- Focus: `InitListExpr`、aggregate initialization、braced initialization
- Complexity: Medium

### P9: Constexpr / Consteval Flow

- Tables: `constexpr_if_initialization`, `constexpr_if_then`, `constexpr_if_else`, `consteval_if_then`, `consteval_if_else`
- Focus: constexpr flow graph，复用并扩展既有 `if_then` / `if_else` 逻辑
- Complexity: Low
- Value: low-risk / high-value phase，适合作为控制流 extractor 的稳定增量。

### P10: Expression Graph Core

- Focus: `exprparents`、expression relationship graph
- Complexity: Medium
- Boundary: 只处理表达式父子关系核心图，不混入 casts、allocation 或 attribute expr argument 的专门语义。

### P11: Casts & Conversion System

- Focus: `conversionkinds`、implicit casts、explicit casts
- Complexity: Medium-High
- Boundary: 与 P0 `derivedtypes` 历史记录保持兼容，后续补全 conversion kind 和显式 cast 语义。

### P12: Allocation & Lifetime System

- Focus: `expr_allocator`、new/delete semantics、allocation relationships
- Complexity: Medium

### P13: Metadata & Misc Cleanup

- Focus: XML metadata、annotations、remaining semantic tail tables
- Complexity: Low-Medium
- Boundary: 仅收尾确实无法归入前述 semantic subsystem 的尾部表，避免重新变成无边界的 catch-all phase。

## Verification Flow

每个 phase 默认使用同一套验证入口。schema / ORM 相关阶段先生成实例化文件，再编译并跑完整测试：

```bash
python3 scripts/generate_instantiations.py
make debug -j8
scripts/test_all.sh
scripts/db_summary.py tests/output/moderate-case.db
```

阶段验收要求：

- 新 subsystem 必要时增加 isolated testcase，避免只依赖综合样例偶然覆盖。
- 高风险阶段建议保留 schema diff、SQL 查询结果或 DB snapshot，作为 review 证据。
- 目标表验收应至少确认 table existence、column shape 和 representative row count。
- 对 dependent type、incomplete definition、ABI/layout、Expr/APValue serialization 等风险点，必须在 phase notes 或 handoff 文档中记录 guard 策略和 deferred 决策。
