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

## Template / Concept Closure

状态：P2/P3 已完成 template/concept 目标表实现收口，当前只保留一个有设计依据的 deferred。

DONE: 90, 91, 96-111 except 112, 113-117.

Deferred by design: 112 `template_template_argument_value`.

`template_template_argument_value.arg_value` is `@expr`, but Clang exposes
template-template arguments as `TemplateName` / `TemplateDecl`, not `Expr*`;
Arborchive records the supported relation through `template_template_argument`
and `template_template_instantiation`.

PR readiness: P2/P3 implemented 21 template/concept target tables; 112 remains
documented deferred.

## P3 Status

| Phase | Status | Notes |
| --- | --- | --- |
| P3a constraint expr extraction | done | Constraint expression support closed for the planned subset. |
| P3b type constraint binding | safe subset complete | Type constraint binding support closed for the planned subset. |
| P3c template-template instantiation semantics | safe subset complete | Process notes: `docs/my-docs/p3/p3c_template_template_semantics.md`. |
| P3d non-type/value template argument expr support | safe subset complete | Process notes: `docs/my-docs/p3/p3d_value_template_argument_support.md`; handoff: `docs/my-docs/p3/p3d_value_template_argument_handoff.md`. |
| P3e template-template argument value feasibility | feasibility complete | Process notes: `docs/my-docs/p3/p3e_template_template_argument_value_feasibility.md`; 112 stays deferred by design. |

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
