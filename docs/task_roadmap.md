# 任务路线图 (Task Roadmap)

本路线图将已知的待办事项（TODOs）分解为适合智能体（agent）处理的、可验证的任务。请保持补丁（patch）小巧，并在完成每个任务后运行 `scripts/test_all.sh`。

## 接下来要完成的 TODO 总览

当前优先级按“先补类型/表达式基础能力，再补声明类能力”的顺序排列：

| 优先级 | TODO | 要完成的行为 | 主要落表 | datatable-list 序号 | 关联/校验表 | 是否需要补实现 datatable-list 表 |
| --- | --- | --- | --- | --- | --- | --- |
| P0 | `ImplicitCastExpr` DerivedType recording | 隐式类型转换不仅记录表达式，还要把转换涉及的派生类型关系写入类型系统 | `derivedtypes` | 81 | `exprs` 176, `builtintypes` 80, `usertypes` 86, `locations_expr` 19 | 否，优先复用现有 `derivedtypes` |
| P1 | Friend declarations | 记录 `friend` 函数、类、模板声明，并保留其声明位置和被引用的函数/类型关系 | `frienddecls` | 144 | `functions` 33, `fun_decls` 46, `type_decls` 64, `usertypes` 86, `member` 138, `locations_default` 17 | 是，当前 datatable-list 已规划 `frienddecls`，仓库尚未实现 |
| P2 | Template declarations | 记录类模板、函数模板、变量模板、模板类型参数，并避免实例化重复写入 | `is_class_template`, `is_function_template`, `is_variable_template` | 96, 102, 106 | `type_decls` 64, `fun_decls` 46, `var_decls` 58, `nontype_template_parameters` 90, `class_template_argument*` 98-99, `function_template_argument*` 104-105, `variable_template_argument*` 108-109 | 是，datatable-list 已规划模板专用表，仓库尚未实现 |

严格约束：

- 不允许自主设计 datatable-list 之外的新表名。
- 不允许使用 `friend_decls`、`template_decls`、`template_parameters` 这类未出现在 `docs/datatable-list.txt` 的替代命名。
- 如果现有仓库没有目标表，只能按 `docs/datatable-list.txt` 的表名和 `docs/semmlecode.cpp.dbscheme` 的字段语义补实现。
- 若确实发现 datatable-list 无法表达某个功能，必须先更新 schema 设计文档并单独评审，不能在功能 patch 中直接新增自定义表。
- 补实现 datatable-list 表时，必须同步更新 `include/model/db/*.h`、`include/db/table_defs/*.h`、`include/db/table_init.h`，并运行 `python3 scripts/generate_instantiations.py`。

说明：

- `datatable-list.txt` 是目标 schema 清单；roadmap 中的序号均对应 `docs/datatable-list.txt`。
- 字段语义优先参考 `docs/semmlecode.cpp.dbscheme`；`docs/scheme_info.md` 可作为辅助，但其中章节编号可能和 `datatable-list.txt` 不完全一致。
- 当前仓库里存在一些中间表或历史表，例如 `types`、`variable`、`locations`，它们不一定在 `datatable-list.txt` 中有同名条目；roadmap 中用它们时会标记为“仓库内部表，非 datatable-list 目标表”。
- `DONE` 表示仓库已实现；无 `DONE` 的表表示 datatable-list 已规划但当前仍待实现。

## 统一验证步骤

每完成一个 TODO，必须执行：

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
```

每完成一个目标表，必须验证：

```bash
sqlite3 tests/output/intense-case.db "SELECT name FROM sqlite_master WHERE type='table' AND name='<table_name>';"
sqlite3 tests/output/intense-case.db "PRAGMA table_info('<table_name>');"
sqlite3 tests/output/intense-case.db "SELECT COUNT(*) FROM <table_name>;"
```

验收说明：

- `sqlite_master` 必须能查到目标表名，且表名必须和 `docs/datatable-list.txt` 完全一致。
- `PRAGMA table_info` 的列名必须对齐 `docs/semmlecode.cpp.dbscheme`。
- row count 不能只看非零；还要用针对性测试样例确认每一行代表正确 AST 语义。
- 不允许通过仓库内部表替代 datatable-list 目标表的验收。

## 任务 1：记录 ImplicitCastExpr 派生类型 (ImplicitCastExpr DerivedType Recording)

状态：待办 (TODO)

要完成的 TODO：

- 完成 `src/core/ast_visitor.cc` 中 `VisitImplicitCastExpr` 附近的 TODO：`record DerivedTypes of it`。
- 当前目标不是新增表达式种类，而是让隐式转换触发类型系统写入。
- 该任务完成后，隐式转换相关类型应能在 SQLite 中通过 `derivedtypes` 查询出来。

当前入口点：

- `src/core/ast_visitor.cc`
- `include/core/ast_visitor.h`
- `src/core/processor/expr_processor.cc`
- `include/core/processor/expr_processor.h`
- `src/core/processor/type_processor.cc`
- `include/core/processor/derivedtype_helper.h`
- `include/model/db/type.h`
- `include/db/table_defs/type.h`

预期变更：

- 让 `ASTVisitor::VisitImplicitCastExpr` 执行的操作多于调用 `expr_processor_->processImplicitCastExpr(ICE)`。
- 确定转换是否应该为隐式转换结果或源类型创建或关联一个 `DerivedType` 行。
- 重用现有的派生类型处理辅助类（helpers）和缓存模式。
- 避免为同一语义类型重复创建类型行。
- 保留现有的 `exprs` 记录行为。

表实现范围：

| datatable-list 序号 | 表 | 状态 | 操作 | 说明 |
| --- | --- | --- | --- | --- |
| 176 | `exprs` | DONE | 复用/保持 | `ExprProcessor::processImplicitCastExpr` 继续记录 `ImplicitCastExpr` 表达式本身。 |
| 81 | `derivedtypes` | DONE | 必须写入或关联 | 记录隐式转换产生的 pointer/reference/qualified/array/function 等派生类型关系。 |
| 80 | `builtintypes` | DONE | 关联 | 隐式算术转换可能关联基础内建类型，例如 `int` 到 `double`。 |
| 86 | `usertypes` | DONE | 关联 | 用户类型到指针、引用、cv-qualified 形式时需要关联基础用户类型。 |
| 19 | `locations_expr` | DONE | 复用 | 保留表达式位置记录，不要求新增 location 表。 |
| 147 | `exprconv` | 未实现 | 暂不作为本任务第一目标 | 如果后续要精确记录“表达式发生了哪种转换”，应实现该表；本任务先完成 DerivedType 写入。 |
| 155 | `conversionkinds` | 未实现 | 暂不作为本任务第一目标 | 如果实现 `exprconv`，需要用该表或等价枚举解释转换种类。 |
| 非 datatable-list | `types` | 仓库内部表 | 复用缓存/避免重复 | 如果当前类型系统仍使用中间表，则通过既有模式关联派生类型和基础类型；不要重复制造相同语义类型。 |

对齐结论：

- 本任务第一阶段严格落到 datatable-list 已 DONE 的 `derivedtypes`(81)，不新增表。
- `docs/datatable-list.txt` 中已经规划了 `exprconv`(147) 和 `conversionkinds`(155)，但这两个表不是本 TODO 的第一阶段目标。
- 如果后续要记录“哪个表达式转换成哪个 cast/conversion 节点”，只能升级为实现 `exprconv`(147) 和 `conversionkinds`(155)，不能新增其他自定义转换表。

相关 dbscheme 字段：

```text
derivedtypes(id, name, kind, type_id)
exprconv(converted, conversion)
conversionkinds(expr_id, kind)
```

建议的测试示例：

```cpp
void implicit_casts() {
  int i = 1;
  double d = i;
  const int *p = &i;
  const void *vp = p;
}
```

验证：

```bash
scripts/test_all.sh
scripts/db_summary.py tests/output/intense-case.db
sqlite3 tests/output/intense-case.db 'SELECT COUNT(*) FROM derivedtypes;'
sqlite3 tests/output/intense-case.db 'SELECT id, name, kind, type_id FROM derivedtypes LIMIT 20;'
sqlite3 tests/output/intense-case.db "PRAGMA table_info('derivedtypes');"
```

验收标准：

- 构建成功。
- `slight-case`、`moderate-case` 和 `intense-case` 都能生成 SQLite 数据库。
- 相关的隐式转换示例能记录预期的 `derivedtypes` 行，且没有明显的重复。
- 现有的 `exprs`、`types` 和 `locations` 统计数据保持被填充。

## 任务 2：友元声明 (Friend Declarations)

状态：待办 (TODO)

要完成的 TODO：

- 完成 `src/core/ast_visitor.cc` 中 `VisitFriendDecl(clang::FriendDecl *decl)` 当前直接 `return true` 的 TODO。
- 该任务需要按 datatable-list 144 实现 `frienddecls`，并复用 `functions`、`fun_decls`、`type_decls`、`usertypes`、`member`、`locations_default` 等既有目标表做关联校验。
- 该任务完成后，friend 函数和 friend 类都应该能在 SQLite 中被定位到声明位置和被引用实体。

当前入口点：

- `src/core/ast_visitor.cc`
- `include/core/ast_visitor.h`
- `src/core/processor/function_processor.cc`
- `src/core/processor/type_processor.cc`
- `include/model/db/declaration.h`
- `include/db/table_defs/declaration.h`
- `include/model/db/class.h`
- `include/db/table_defs/class.h`

预期变更：

- 实现 `ASTVisitor::VisitFriendDecl`。
- 决定友元函数、友元类或友元模板是否应委托给现有的函数/类型/声明处理流程。
- 使用现有的声明/容器/类（declaration/container/class）模式记录位置和声明关系。
- 如果当前仓库没有 `frienddecls`，必须按 datatable-list 144 补实现这个表；不允许新增其他 friend 表名。

表实现范围：

| datatable-list 序号 | 表 | 状态 | 操作 | 说明 |
| --- | --- | --- | --- | --- |
| 144 | `frienddecls` | 未实现 | 必须实现 | 这是 friend 声明对应的目标表；本 TODO 的主交付就是补上该表及写入路径。 |
| 33 | `functions` | DONE | 关联/必要时写入 | friend function declaration 应关联到函数实体，不应重复创建已有函数。 |
| 46 | `fun_decls` | DONE | 关联/必要时写入 | friend function declaration 应关联到函数声明和位置。 |
| 64 | `type_decls` | DONE | 关联/必要时写入 | friend class declaration 应关联到类型声明。 |
| 86 | `usertypes` | DONE | 关联 | friend class 指向的类/结构体应能回到用户类型。 |
| 138 | `member` | DONE | 可能关联 | 若 friend 被视为类上下文中的成员级关系，可用来校验声明所属类。 |
| 17 | `locations_default` | DONE | 必须写入或复用 | friend 声明需要可追溯源代码位置。 |
| 非 datatable-list | `declarations` | 仓库内部表 | 仅辅助 | 当前仓库有 `declarations` 表，但 datatable-list 中没有同名目标表；不要用它替代 `frienddecls` 的最终交付。 |
| 非 datatable-list | `types` | 仓库内部表 | 仅辅助 | 当前仓库类型中间表，不是 datatable-list 目标表。 |

本 TODO 要补实现的 datatable-list 表：

| datatable-list 序号 | 表 | 目的 | dbscheme 字段 |
| --- | --- | --- | --- |
| 144 | `frienddecls` | 记录 friend 声明节点本身，并关联 friend 指向的类型/声明和源码位置 | `id`, `type_id`, `decl_id`, `location` |

补表验收条件：

- 必须优先实现 datatable-list 中已有的 `frienddecls`(144)，不要另起 `friend_decls` 这种不同命名。
- `frienddecls` 字段必须对齐 `docs/semmlecode.cpp.dbscheme`：`id`, `type_id`, `decl_id`, `location`。
- `frienddecls` 必须能回答：friend 声明节点是什么，friend 指向哪个类型或声明，源码位置在哪里。
- 如果实现时发现需要上下文信息，先复用 datatable-list 已有关系表，例如 `member`(138)、`namespacembrs`(152) 或 location/container 相关表；不要擅自新增 datatable-list 之外的表。

建议的测试示例：

```cpp
class Secret;

class Box {
  friend void inspect(Box &);
  friend class Secret;
  int value;
};

void inspect(Box &) {}
```

验证：

```bash
scripts/test_all.sh
scripts/db_summary.py tests/output/moderate-case.db
sqlite3 tests/output/moderate-case.db "SELECT name FROM sqlite_master WHERE type='table' AND name='frienddecls';"
sqlite3 tests/output/moderate-case.db "PRAGMA table_info('frienddecls');"
sqlite3 tests/output/moderate-case.db 'SELECT COUNT(*) FROM frienddecls;'
sqlite3 tests/output/moderate-case.db 'SELECT id, type_id, decl_id, location FROM frienddecls LIMIT 20;'
```

验收标准：

- 访问了友元声明，并通过现有的或新记录的模式（schema）持久化。
- `frienddecls`(144) 存在，列名对齐 dbscheme。
- 友元函数/类不创建重复的主要函数/类型记录。
- 该实现处理了友元函数和友元类两种形式。

## 任务 3：模板声明 (Template Declarations)

状态：待办 (TODO)

要完成的 TODO：

- 完成 `src/core/ast_visitor.cc` 中 `VisitTemplateDecl(clang::TemplateDecl *decl)` 当前直接 `return true` 的 TODO。
- 该任务需要让模板声明分发到现有函数、类型、变量处理器，并明确模板声明与实例化之间的去重策略。
- 该任务完成后，类模板、函数模板、变量模板、模板类型参数应该能通过函数/类型/变量相关表查询到。

当前入口点：

- `src/core/ast_visitor.cc`
- `include/core/ast_visitor.h`
- `src/core/processor/type_processor.cc`
- `include/core/processor/type_processor.h`
- `src/core/processor/function_processor.cc`
- `include/core/processor/function_processor.h`
- `src/util/key_generator/type.cc`
- `include/util/key_generator/type.h`
- `include/model/db/type.h`
- `include/db/table_defs/type.h`
- `include/model/db/function.h`
- `include/db/table_defs/function.h`

预期变更：

- 实现 `ASTVisitor::VisitTemplateDecl`。
- 将类模板、函数模板、变量模板和模板类型参数分发给拥有底层声明的现有处理器。
- 在适用的地方重用 `KeyGen::Type::makeKey(const TemplateDecl *, ASTContext *)`。
- 避免重复处理模板实例化，因为 `shouldVisitTemplateInstantiations()` 已经返回 `true`。
- 保持模板参数和模板参数（arguments）的记录与当前类型/函数模式一致。

表实现范围：

| datatable-list 序号 | 表 | 状态 | 操作 | 说明 |
| --- | --- | --- | --- | --- |
| 96 | `is_class_template` | 未实现 | 必须实现 | 标记某个类型/类声明是类模板。 |
| 97 | `class_instantiation` | 未实现 | 必要时实现 | 记录类模板实例化，例如 `Holder<int>`。 |
| 98 | `class_template_argument` | 未实现 | 必要时实现 | 记录类模板实参。 |
| 99 | `class_template_argument_value` | 未实现 | 必要时实现 | 记录类模板非类型实参值。 |
| 102 | `is_function_template` | 未实现 | 必须实现 | 标记某个函数声明是函数模板。 |
| 103 | `function_instantiation` | 未实现 | 必要时实现 | 记录函数模板实例化，例如 `identity<int>`。 |
| 104 | `function_template_argument` | 未实现 | 必要时实现 | 记录函数模板实参。 |
| 105 | `function_template_argument_value` | 未实现 | 必要时实现 | 记录函数模板非类型实参值。 |
| 106 | `is_variable_template` | 未实现 | 必要时实现 | 标记变量模板。 |
| 107 | `variable_instantiation` | 未实现 | 必要时实现 | 记录变量模板实例化。 |
| 108 | `variable_template_argument` | 未实现 | 必要时实现 | 记录变量模板实参。 |
| 109 | `variable_template_argument_value` | 未实现 | 必要时实现 | 记录变量模板非类型实参值。 |
| 110 | `template_template_instantiation` | 未实现 | 后续扩展 | 记录 template-template 参数实例化。 |
| 111 | `template_template_argument` | 未实现 | 后续扩展 | 记录 template-template 实参。 |
| 112 | `template_template_argument_value` | 未实现 | 后续扩展 | 记录 template-template 实参值。 |
| 90 | `nontype_template_parameters` | 未实现 | 必要时实现 | 记录非类型模板参数。 |
| 91 | `type_template_type_constraint` | 未实现 | 后续扩展 | 记录模板类型约束。 |
| 113-117 | `concept_templates`, `concept_instantiation`, `is_type_constraint`, `concept_template_argument`, `concept_template_argument_value` | 未实现 | 后续扩展 | concept template/instantiation/constraint/argument 相关表，不作为第一轮模板声明 TODO 的主目标。 |
| 33 | `functions` | DONE | 关联/必要时写入 | 函数模板底层 `FunctionDecl` 应进入函数实体表。 |
| 46 | `fun_decls` | DONE | 关联/必要时写入 | 函数模板底层声明应进入函数声明表。 |
| 64 | `type_decls` | DONE | 关联/必要时写入 | 类模板、模板特化、模板类型参数应进入类型声明体系。 |
| 86 | `usertypes` | DONE | 关联 | 类模板底层类/结构体应能回到用户类型。 |
| 58 | `var_decls` | DONE | 必要时写入或关联 | 变量模板应进入变量声明体系。 |
| 72 | `params` | DONE | 关联 | 函数模板参数列表中的函数参数仍由现有参数表记录。 |
| 118 | `routinetypes` | DONE | 关联 | 函数模板类型签名应复用 routine type 结构。 |
| 119 | `routinetypeargs` | DONE | 关联 | 函数模板参数类型应复用 routine type args。 |
| 17 | `locations_default` | DONE | 必须写入或复用 | 模板声明需要可追溯源代码位置。 |
| 非 datatable-list | `types` | 仓库内部表 | 仅辅助 | 当前仓库类型中间表，不是 datatable-list 目标表。 |
| 非 datatable-list | `variable` | 仓库内部表 | 仅辅助 | 当前仓库变量中间表，不是 datatable-list 目标表。 |

本 TODO 要补实现的 datatable-list 表：

| datatable-list 序号 | 表 | 第一轮优先级 | dbscheme 字段 | 目的 |
| --- | --- | --- | --- | --- |
| 96 | `is_class_template` | P0 | `id` | 标记类模板声明。 |
| 102 | `is_function_template` | P0 | `id` | 标记函数模板声明。 |
| 106 | `is_variable_template` | P1 | `id` | 标记变量模板声明。 |
| 90 | `nontype_template_parameters` | P1 | `id` | 记录非类型模板参数。 |
| 97 | `class_instantiation` | P1 | `to`, `from` | 记录类模板实例化来源关系。 |
| 98 | `class_template_argument` | P1 | `type_id`, `index`, `arg_type` | 记录类模板类型实参。 |
| 99 | `class_template_argument_value` | P1 | `type_id`, `index`, `arg_value` | 记录类模板非类型实参值。 |
| 103 | `function_instantiation` | P1 | `to`, `from` | 记录函数模板实例化来源关系。 |
| 104 | `function_template_argument` | P1 | `function_id`, `index`, `arg_type` | 记录函数模板类型实参。 |
| 105 | `function_template_argument_value` | P1 | `function_id`, `index`, `arg_value` | 记录函数模板非类型实参值。 |
| 107 | `variable_instantiation` | P2 | `to`, `from` | 记录变量模板实例化来源关系。 |
| 108 | `variable_template_argument` | P2 | `variable_id`, `index`, `arg_type` | 记录变量模板类型实参。 |
| 109 | `variable_template_argument_value` | P2 | `variable_id`, `index`, `arg_value` | 记录变量模板非类型实参值。 |
| 110 | `template_template_instantiation` | P2 | `to`, `from` | 记录 template-template 实例化来源关系。 |
| 111 | `template_template_argument` | P2 | `type_id`, `index`, `arg_type` | 记录 template-template 类型实参。 |
| 112 | `template_template_argument_value` | P2 | `type_id`, `index`, `arg_value` | 记录 template-template 非类型实参值。 |
| 91 | `type_template_type_constraint` | P2 | `id`, `constraint` | 记录模板类型约束。 |
| 113 | `concept_templates` | P2 | `concept_id`, `name`, `location` | 记录 concept 模板。 |
| 114 | `concept_instantiation` | P2 | `to`, `from` | 记录 concept 实例化来源关系。 |
| 115 | `is_type_constraint` | P2 | `concept_id` | 标记 type constraint。 |
| 116 | `concept_template_argument` | P2 | `concept_id`, `index`, `arg_type` | 记录 concept 类型实参。 |
| 117 | `concept_template_argument_value` | P2 | `concept_id`, `index`, `arg_value` | 记录 concept 非类型实参值。 |

补表验收条件：

- 优先实现 datatable-list 已规划的模板表，不新增 `template_decls` / `template_parameters` 这类自定义命名表。
- 第一轮至少要让 `is_class_template`(96) 和 `is_function_template`(102) 可落表。
- 若变量模板也在同一 patch 覆盖，则同步实现 `is_variable_template`(106)。
- 实例化和模板实参表可以分阶段实现，但每一阶段必须在 roadmap 或 patch 说明里标明已完成哪些 datatable-list 序号。

建议的测试示例：

```cpp
template <typename T>
T identity(T value) {
  return value;
}

template <typename T>
struct Holder {
  T value;
};

void use_templates() {
  Holder<int> h{identity(1)};
}
```

验证：

```bash
scripts/test_all.sh
scripts/db_summary.py tests/output/intense-case.db
sqlite3 tests/output/intense-case.db 'SELECT COUNT(*) FROM type_decls;'
sqlite3 tests/output/intense-case.db 'SELECT COUNT(*) FROM fun_decls;'
sqlite3 tests/output/intense-case.db "SELECT name FROM sqlite_master WHERE type='table' AND name IN ('is_class_template','is_function_template','is_variable_template','class_instantiation','function_instantiation','variable_instantiation') ORDER BY name;"
sqlite3 tests/output/intense-case.db "PRAGMA table_info('is_class_template');"
sqlite3 tests/output/intense-case.db "PRAGMA table_info('is_function_template');"
sqlite3 tests/output/intense-case.db 'SELECT COUNT(*) FROM is_class_template;'
sqlite3 tests/output/intense-case.db 'SELECT COUNT(*) FROM is_function_template;'
sqlite3 tests/output/intense-case.db 'SELECT id FROM is_class_template LIMIT 20;'
sqlite3 tests/output/intense-case.db 'SELECT id FROM is_function_template LIMIT 20;'
```

验收标准：

- 访问了模板声明并委托给现有的处理器。
- 第一轮必须至少实现 `is_class_template`(96) 与 `is_function_template`(102)，表名和列名对齐 dbscheme。
- 模板函数和模板类出现在函数/类型统计中。
- 实例化的形式不会导致行数意外激增。
- 现有测试仍然通过。
