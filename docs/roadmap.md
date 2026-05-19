# Arborchive Roadmap

本文件记录 Arborchive 当前阶段性进展和下一阶段方向。详细 schema 以
`docs/datatable-list.txt` 为准；字段语义优先参考
`docs/semmlecode.cpp.dbscheme`。

## 工作边界

- 小步 patch，保持每一步可回滚、可验证。
- 不新增 `docs/datatable-list.txt` 之外的表名。
- 不用 `friend_decls`、`template_decls`、`template_parameters` 这类自定义表名替代目标表。
- 修改 schema / ORM 时同步更新 model、table_defs、table_init，并运行 `python3 scripts/generate_instantiations.py`。
- 每次功能 patch 后优先运行 `scripts/test_all.sh`，并检查生成的 SQLite 数据库。

## 已完成阶段记录

| Phase | Theme | Main tables | Commits | Status |
| --- | --- | --- | --- | --- |
| P0 DerivedTypes | record derived types for implicit casts | `derivedtypes`(81) | `a3ac8e6` | done / verified |
| P1 frienddecls | complete frienddecls 144 | `frienddecls`(144) | `5b07c13` | done / verified |
| P2 template system phase | template declaration markers, instantiations, arguments, value extraction safe subset, variable templates, template-template arguments, concept templates | 96-99, 102-109, 111, 113, 114, 116 | `9f395d6`, `f655a10`, `2e02b8a`, `b93ffa6`, `9770870`, `eb7035f`, `9a2c2fc`, `d6323a2` | stage complete |

## TODO3 / P2e Template System Closure

状态：阶段性完成。P2e-4 只做收口验证和文档对齐，不继续补表或扩展抽取逻辑。

DONE: 96-99, 102-109, 111, 113, 114, 116.

Deferred by design: 90, 91, 110, 112, 115, 117. 这些表依赖更明确的
constraint expr、真实 source `Expr*`、template-template instantiation 语义或
TemplateTemplateParmDecl 建模链路，留到 P3。

当前限制：dependent array size 保守写 0；`*_template_argument_value` 只在有真实
source `Expr*` 并能安全落到 `@expr` 时写入。

## 下一阶段路线

P3 不是继续机械补表，而是处理 P2 deferred 背后的语义建模问题：
constraint expr extraction、type constraint binding、template-template
instantiation semantics、non-type/value template argument expr support。

| Phase | 目标 | Deferred | 边界 |
| --- | --- | --- | --- |
| P3a concept/constraint expr extraction | 调查并支持 concept / requires / type constraint 相关表达式能否安全落到 `@expr`。 | 91, 115, 117 | investigation complete；详见 `docs/p3a_constraint_expr_investigation.md`。 |
| P3b type constraint binding | 明确 template type parameter 与 concept/type constraint 的绑定关系。 | 91, 115 | partial safe subset implemented；`template <Concept T>` 可写 91/115，其他 constraint/value 场景继续 deferred。 |
| P3c template-template instantiation semantics | 明确 `template_template_instantiation` 的 from/to 语义，以及 TemplateTemplateParmDecl -> `@usertype(kind=8)` 建模链路。 | 110, 112 | 先定义语义和 fixture，不伪造 TemplateName / TemplateDecl 为 `Expr*`。 |
| P3d non-type/value template argument expr support | 扩大 non-type/value template argument 中真实 source `Expr*` 的安全支持范围。 | 90, 112, 117 | 继续遵守 P2 限制：`*_template_argument_value` 只在有真实 source `Expr*` 且能安全落到 `@expr` 时写入。 |

## 验证入口

常规验证：

```bash
scripts/test_all.sh
scripts/db_summary.py tests/output/moderate-case.db
```

schema / ORM 变更额外运行：

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
```
