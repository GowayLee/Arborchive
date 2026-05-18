# 智能体工作流 (Agents Workflow)

本文档描述了修改 Arborchive 的智能体（agents）应该遵循的工作流程。

## 在修改代码之前

首先阅读以下文件：

- `AGENTS.md`
- `src/core/ast_visitor.cc`
- `include/core/ast_visitor.h`
- 位于 `include/core/processor/` 和 `src/core/processor/` 下的相关处理器（processor）头文件和源文件
- 位于 `include/model/db/` 和 `include/db/table_defs/` 下的相关模型（model）和表定义（table definition）
- 位于 `include/util/key_generator/` 和 `src/util/key_generator/` 下的任何相关键生成器（key generators）
- 在处理当前的待办事项时阅读 `docs/task_roadmap.md`

对于持久化或模式（schema）相关的工作，还要阅读：

- `include/db/table_init.h`
- `include/db/storage_facade.h`
- `src/db/storage_facade.cc`
- `src/db/storage_facade_instantiations.inc`
- `scripts/generate_instantiations.py`

## 处理器修改工作流 (Processor Change Workflow)

1. 在 `ASTVisitor` 中确定 Clang 节点入口点。
2. 保持访问者（visitor）方法精简；将实际行为委托给处理器（processor）。
3. 在 `include/core/processor/<processor>.h` 中添加处理器方法声明。
4. 在 `src/core/processor/<processor>.cc` 中实现该方法。
5. 在添加新的辅助类（helpers）之前，先使用现有的辅助类。
6. 当目标领域已经这样做时，使用稳定的键生成（key generation）和缓存查找（cache lookup）。
7. 通过现有的位置（location）管道记录源代码位置。
8. 通过 `STG` 插入数据库行。
9. 如果现有测试用例未覆盖该行为，在 `tests/` 下添加一个最小化的测试输入。
10. 运行必需的验证命令。

## 模型/表定义/ORM 工作流 (Model/TableDefs/ORM Workflow)

1. 更新 `include/model/db/<domain>.h`。
2. 更新 `include/db/table_defs/<domain>.h`。
3. 当添加新表时，在 `include/db/table_init.h` 中包含该表。
4. 如果新模型需要显式的插入/更新（insert/update）支持，更新存储外观（storage facade）的声明/定义。
5. 重新生成 ORM 实例化：

```bash
python3 scripts/generate_instantiations.py
```

6. 构建并运行测试：

```bash
make debug -j 8
scripts/test_all.sh
```

7. 检查生成的数据库：

```bash
scripts/db_summary.py tests/output/slight-case.db
scripts/db_summary.py tests/output/moderate-case.db
scripts/db_summary.py tests/output/intense-case.db
```

## 每次提供补丁后必需的验证

最低要求：

```bash
make debug -j 8
scripts/test_all.sh
```

如果模式/模型/表定义（schema/model/table definitions）发生了更改：

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
```

如果一个待办任务（TODO task）改变了输出语义，还要运行针对性的 SQLite 检查，例如：

```bash
scripts/db_summary.py tests/output/intense-case.db
sqlite3 tests/output/intense-case.db 'SELECT COUNT(*) FROM derivedtypes;'
```

## 常见错误 (Common Failures)

### LLVM 工具链不匹配 (LLVM Toolchain Mismatch)

症状：`make` 因为 LLVM 版本或 Apple Clang 错误而退出。

调试：

```bash
make print-toolchain
```

通过让 make 指向 LLVM 19 来修复：

```bash
make LLVM_CONFIG=/path/to/llvm-config-19 CXX=/path/to/clang++ debug -j 8
```

### 模式变更后出现 ORM 或链接错误 (ORM Or Link Errors After Schema Changes)

症状：缺少模板实例化、存储方法未定义或表/模型不匹配。

调试：

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
```

检查模型字段、表列、表初始化和存储外观（storage facade）声明是否一致。

### 预期有数据的表为空 (Empty Expected Tables)

症状：构建和运行成功，但核心表有零行数据。

调试：

```bash
scripts/db_summary.py tests/output/<case>.db
```

然后检查：

- `src/core/ast_visitor.cc` 中的 `Visit*` 方法
- 目标处理器方法
- 缓存键的生成和提前返回逻辑
- 在插入之前，位置或依赖解析是否失败

### 测试数据库未创建 (Test Database Not Created)

症状：`scripts/test_all.sh` 在运行一个用例后退出。

调试：

```bash
./build/demo -c ./config.example.toml -s ./tests/slight-case.cc -o ./tests/output/slight-case.db
```

检查运行时日志并确保 `tests/output/` 是可写的。

### SQLite 列名意外 (SQLite Column Name Surprises)

保留了一些历史的表名/列名，包括诸如 `associated_ed` 的名字。不要随意重命名列名；模式的变化会导致下游成本。
