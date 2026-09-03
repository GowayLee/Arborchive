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
| P11 casts / conversions | conversion graph, expression types/categories, loads, implicit/explicit cast semantics | `exprconv`(147), `compgenerated`(148), `expr_isload`(154), `conversionkinds`(155), `expr_types`(178) | `5808032`, `701fca1`, `c8af902` | done / verified |

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

- Status: PARTIAL. P7a/P7b and the P7c/P7d/P7e owner-link safe subsets are
  implemented, but the full attribute system is not complete and should not be
  marked `DONE` yet.
- Implemented safe subset: function attribute presence; `DeprecatedAttr` stable
  string payloads, `AnnotateAttr` annotation strings, `SectionAttr` section
  names, and `WarnUnusedResultAttr` / `[[nodiscard("...")]]` messages in
  `attribute_arg_value`; direct and shallow wrapped integer-literal
  `AlignedAttr` payloads in `attribute_arg_constant`; minimal non-literal
  `AlignedAttr` and `AssumeAlignedAttr` expression subsets in
  `attribute_arg_expr`; conservative type, variable, and statement owner links
  in `typeattributes`, `varattributes`, and `stmtattributes`.
- Deferred: P7f `attribute_arg_type`, `attribute_arg_name`, broader
  `attribute_arg_expr` families such as `EnableIfAttr`, `DiagnoseIfAttr`, and
  `AnnotateAttr` expression args, generalized `CONSTANT_EXPR`,
  dependent/template attribute arguments, named arguments, target-unsupported
  alias strings, availability named fields, and other complex argument forms.
- Analysis: `docs/analysis/p7_attribute_owner_links.md` and
  `docs/analysis/p7_remaining_attribute_scope_audit.md`.

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
- Status: DONE (P8 safe subset)
- Implemented scope: `VarDecl::getInit()` variable initializers, braced initializer marker rows, and aggregate child initializer dependency resolution.
- Validation: `tests/unit-tests/p8/initialization_case.cc` via `scripts/test_all.sh`; representative SQL confirmed `initialisers`, `braced_initialisers`, `aggregate_field_init`, and `aggregate_array_init` rows.
- Boundary: constructor initializers、member initializers、default member initializers remain out of scope for P8.

### P9: Constexpr / Consteval Flow

- Tables: `constexpr_if_initialization`, `constexpr_if_then`, `constexpr_if_else`, `consteval_if_then`, `consteval_if_else`
- Focus: constexpr flow graph，复用并扩展既有 `if_then` / `if_else` 逻辑
- Complexity: Low
- Value: low-risk / high-value phase，适合作为控制流 extractor 的稳定增量。
- Status: DONE
- Implemented scope: ordinary / constexpr / consteval / negated consteval
  `IfStmt` classification, CodeQL-aligned relationship tables, and statement
  dependency reuse for initializer, then, and else children, including
  declaration and expression init-statements.
- Validation: `tests/unit-tests/p9/constexpr_flow_case.cc` is parsed as C++23
  and checked by `scripts/test_all.sh` with SQLite schema, kind, uniqueness, and
  reference assertions.
- Migration: the historical `if_initalization` output table was replaced by
  the CodeQL-compatible `if_initialization` spelling; old databases must be
  regenerated.

### P10: Expression Graph Core

- Focus: `exprparents`、expression relationship graph
- Complexity: Medium
- Boundary: 只处理表达式父子关系核心图，不混入 casts、allocation 或 attribute expr argument 的专门语义。
- Status: DONE
- Scope: `exprparents(expr_id, child_index, parent_id)` 连接已支持的
  expression、statement 与 P8 initialiser；主表达式树通过
  `IgnoreParenCasts()` 排除 conversion wrapper，并保留 CodeQL 的 call、
  control-flow 与 initialiser 子节点索引。
- Architecture: `ExprProcessor` 持有图关系、规范化与去重语义，
  `StmtProcessor`/`InitializationProcessor` 只通过显式 API 写边；
  `ASTVisitor` 只注入依赖并继续派发。
- Validation: `tests/unit-tests/p10/expression_graph_case.cc` 由
  `scripts/test_all.sh` 执行，SQLite 断言覆盖列形状、引用完整性、唯一主父、
  conversion 边界及代表性 child index。
- Migration: 新表不改变已有字段；旧数据库需重新生成以获得
  `exprparents`。

### P11: Casts & Conversion System

- Focus: `conversionkinds`、implicit casts、explicit casts
- Complexity: Medium-High
- Status: DONE
- Scope: 新增 CodeQL 对齐的 `exprconv`、`expr_types`、`expr_isload`、
  `compgenerated`、`conversionkinds`。`exprconv` 固定从被转换表达式指向
  wrapper；`exprparents` 继续排除所有 conversion wrapper。
- Implicit conversions: `CK_LValueToRValue` 只标记主表达式 load；数组衰减
  使用 kind 8；其余支持范围内的非 dependent 隐式 cast 使用 kind 214，
  并写入 `conversionkinds`。隐式 wrapper 写 `compgenerated`。
- Explicit conversions: `static_cast`/`reinterpret_cast`/`const_cast`/
  `dynamic_cast`/C-style 与 functional cast 分别使用 210–214；每个显式
  cast 恰有一个 `conversionkinds` 行且不写 `compgenerated`。
- Expression metadata: 每个受支持表达式写入一个 `expr_types` 行，值类别
  固定为 prvalue 1、xvalue 2、lvalue 3。仅 conversion key 追加 AST 类别、
  CastKind、源/目标类型和值类别，非 conversion key 保持原行为。
- Validation: `tests/unit-tests/p11/casts_conversion_case.cc` 与
  `scripts/assert_test_db.py` 覆盖嵌套链、load、数组衰减、bool/数值、继承、
  member pointer、五类显式 cast、值类别、引用完整性和 conversion/main-tree
  隔离；LLVM 19 build 与 `scripts/test_all.sh` 通过。
- Deferred: dependent cast、`reference_to`、`ref_indirect`、`temp_init`、
  C11 generic、ObjC/address-space 专用转换和 `BuiltinBitCastExpr`。
- Migration: 支持范围内原 kind 217 隐式 cast 改为 CodeQL cast kind 214；
  schema 新增五张表，旧数据库必须重建。

### P12: Allocation & Lifetime System

- Focus: `expr_allocator`、new/delete semantics、allocation relationships
- Complexity: Medium
- Status: DONE
- Scope: `CXXNewExpr`/`CXXDeleteExpr` 分别写入 kind 87/129 与 88/128，
  并新增 CodeQL 对齐的 `new_allocated_type`、
  `new_array_allocated_type`、`expr_allocator`、`expr_deallocator`。
- Architecture: allocation semantics 与主图安全子集由 `ExprProcessor`
  持有；`FunctionProcessor::resolveFunctionReference()` 只提供窄化、按需的
  稳定函数 ID 解析；`ASTVisitor` 仅注入依赖并派发 new/delete AST 节点。
- Graph: initializer、动态外层 extent、trivially destructible delete
  operand 分别使用 child 1、2、3；不合成 allocator/deallocator/destructor
  call expression。
- Lifetime closure: `synthetic_destructor_call`、`expr_reuse` 与 delete 合成
  call 子节点继续 deferred；重新进入条件记录于
  `.trellis/tasks/p12-allocation-lifetime/research.md`。
- Validation: `tests/unit-tests/p12/allocation_lifetime_case.cc` 覆盖普通/数组、
  常量/动态/嵌套数组、placement、over-aligned、类级 operator、
  sized/aligned/destroying delete 与虚析构 guard，并由 SQLite 断言检查
  schema、kind、类型、form、引用完整性及主图边。
- Migration: 新增四张表并开始输出四种 new/delete kind；旧数据库必须重建。

### P13: Metadata & Misc Cleanup

- Focus: compilation metadata and function documentation-comment safe subset
- Complexity: Low-Medium
- Status: DONE (metadata/comment safe subset)
- Boundary: 仅收尾确实无法归入前述 semantic subsystem 的尾部表，避免重新变成无边界的 catch-all phase。
- P13a: 新增 `compilation_compiling_files` 与 `extractor_version`；单文件关系使用
  `num=0`，手动模式使用 `mode=1`，`compilation_finished.id` 复用
  compilation ID。实际 Clang 执行和 `compilation_args` 共享同一完整参数
  序列；解析失败不写 finished。
- P13b: `CommentProcessor` 仅持久化主文件中 Clang 稳定绑定到显式
  function declaration/definition 的 documentation comments；内容使用
  Clang formatted text，location 使用 raw comment source range，并按 canonical
  declaration + source range 去重。`FunctionProcessor` 持有绑定协作，
  `VisitFunctionDecl` 没有新增 comments 跨表编排。
- Deferred: 普通未绑定注释、宏注释、variable/type/statement 注释绑定、
  `fileannotations` 以及全部 `xml*` 表。XML 重新进入条件见
  `docs/analysis/p13_xml_deferred.md`；P7、CFG、VLA、name qualifier、link、
  diagnostic 等继续归属各自语义子系统。
- Validation: `tests/unit-tests/p13/metadata_comments_case.cc` 和 SQLite 断言
  覆盖参数序列、ID/引用完整性、版本、注释文本/位置/绑定/去重及
  deferred 表缺席。
- Migration: 新增四张 CodeQL-compatible 表，旧数据库需重建。

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
