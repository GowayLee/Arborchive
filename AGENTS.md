# AGENTS.md

Arborchive 的 agent 总控入口。更完整的项目百科保存在 `docs/AGENTS_GUIDE.md`；具体工作流看 `docs/agent_workflow.md`；当前 TODO 路线图看 `docs/task_roadmap.md`。

## 默认规则

- 默认用中文回复。
- 小步 patch，保持每一步可回滚、可验证。
- 不做大规模 C++ 重构，除非用户明确要求。
- 不删除既有 C++ 代码；必要时先保留上下文并解释原因。
- 修改前先说明计划；修改后必须说明 changed files、validation commands、test results、remaining risks。
- 生成的测试数据库统一放在 `tests/output/`。

## 修改前必读

开始改 C++ 行为前，按顺序读：

1. `AGENTS.md`
2. `docs/agent_workflow.md`
3. `docs/task_roadmap.md`
4. `src/core/ast_visitor.cc`
5. 相关 Processor 的头文件和实现：
   - `include/core/processor/<name>_processor.h`
   - `src/core/processor/<name>_processor.cc`
6. 相关 DB model 和 table definition：
   - `include/model/db/<domain>.h`
   - `include/db/table_defs/<domain>.h`
7. 相关 key generator：
   - `include/util/key_generator/`
   - `src/util/key_generator/`

如果修改 schema / ORM，还必须读：

- `docs/datatable-list.txt`
- `docs/semmlecode.cpp.dbscheme`
- `include/db/table_init.h`
- `include/db/storage_facade.h`
- `src/db/storage_facade.cc`
- `scripts/generate_instantiations.py`
- `src/db/storage_facade_instantiations.inc`

## Schema 硬约束

- 不允许自主新增 `docs/datatable-list.txt` 之外的新表。
- 表名必须严格使用 `docs/datatable-list.txt` 中的命名。
- 字段语义优先参考 `docs/semmlecode.cpp.dbscheme`。
- 不允许用 `friend_decls`、`template_decls`、`template_parameters` 这类自定义命名替代目标表。
- 补实现 datatable-list 表时，必须同步更新 model、table_defs、table_init，并运行 ORM 生成脚本。

## 当前 TODO

当前只优先推进 `docs/task_roadmap.md` 中的任务：

- P0: `ImplicitCastExpr` DerivedType recording，主要落 `derivedtypes`(81)。
- P1: Friend declarations，必须实现 `frienddecls`(144)。
- P2: Template declarations，必须按 datatable-list 90-117 的模板表推进，第一轮至少覆盖 `is_class_template`(96) 和 `is_function_template`(102)。

不要把这些任务扩展成无关重构。

## 默认验证

每次 patch 后优先跑：

```bash
scripts/test_all.sh
```

它会执行：

```bash
make debug -j 8
./build/demo -c ./config.example.toml -s ./tests/slight-case.cc -o ./tests/output/slight-case.db
./build/demo -c ./config.example.toml -s ./tests/moderate-case.cc -o ./tests/output/moderate-case.db
./build/demo -c ./config.example.toml -s ./tests/intense-case.cc -o ./tests/output/intense-case.db
```

检查数据库摘要：

```bash
scripts/db_summary.py tests/output/slight-case.db
scripts/db_summary.py tests/output/moderate-case.db
scripts/db_summary.py tests/output/intense-case.db
```

如果修改 schema / ORM，先跑：

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
```

目标表验收示例：

```bash
sqlite3 tests/output/intense-case.db "SELECT name FROM sqlite_master WHERE type='table' AND name='<table_name>';"
sqlite3 tests/output/intense-case.db "PRAGMA table_info('<table_name>');"
sqlite3 tests/output/intense-case.db "SELECT COUNT(*) FROM <table_name>;"
```

## 常见环境问题

Arborchive 当前要求 LLVM 19。若 `scripts/test_all.sh` 失败并提示 `LLVM_CONFIG` 是 22.x，先指定 LLVM 19：

```bash
export LLVM19_HOME="/opt/homebrew/opt/llvm@19"
export PATH="$LLVM19_HOME/bin:$PATH"
export LLVM_CONFIG="$LLVM19_HOME/bin/llvm-config"
export CC="$LLVM19_HOME/bin/clang"
export CXX="$LLVM19_HOME/bin/clang++"
scripts/test_all.sh
```

也可以先检查：

```bash
make print-toolchain
```

## 代码路径速查

- AST 分发：`src/core/ast_visitor.cc`
- Processor：`src/core/processor/`
- Processor headers：`include/core/processor/`
- DB models：`include/model/db/`
- Table definitions：`include/db/table_defs/`
- ORM table init：`include/db/table_init.h`
- Storage facade：`include/db/storage_facade.h`, `src/db/storage_facade.cc`
- Key generators：`include/util/key_generator/`, `src/util/key_generator/`
- 测试输入：`tests/slight-case.cc`, `tests/moderate-case.cc`, `tests/intense-case.cc`

