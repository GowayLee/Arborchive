# Arborchive Scheme Information

> from CodeQL C/C++ semmelcode dbscheme

### 1. **compilations** :DONE
- **描述**: 记录编译器调用的基本信息。
- **属性**:
  - `id` (INTEGER, 主键): 编译器调用的唯一标识符。
  - `cwd` (TEXT, 外键): 调用编译器时的工作目录。
- **示例**:
  ```sql
  CREATE TABLE compilations (
      id INTEGER PRIMARY KEY,
      cwd TEXT
  );
  ```

### 2. **compilation_args** :DONE
- **描述**: 记录传递给编译器的参数。
- **属性**:
  - `id` (INTEGER, 外键): 编译器调用的ID，引用自`compilations`表。
  - `num` (INTEGER): 参数的序号。
  - `arg` (TEXT): 参数的内容。
- **主键**: (`id`, `num`)
- **示例**:

  ```sql
  CREATE TABLE compilation_args (
      id INTEGER,
      num INTEGER,
      arg TEXT,
      PRIMARY KEY (id, num),
      FOREIGN KEY (id) REFERENCES compilations(id)
  );
  ```

  For command `$gcc -c f1.c f2.c f3.c`

  ```
  | num | arg                 |
  | --- | ------------------- |
  | 0   | *path to extractor* |
  | 1   | `--mimic`           |
  | 2   | `/usr/bin/gcc`      |
  | 3   | `-c`                |
  | 4   | f1.c                |
  | 5   | f2.c                |
  | 6   | f3.c                |
  ```

### 3. **compilation_build_mode** :DONE
- **描述**: 可选地记录每次编译的构建模式。
- **属性**:
  - `id` (INTEGER, 外键): 编译器调用的ID，引用自`compilations`表。
  - `mode` (INTEGER): 构建模式的标识符。
- **示例**:
  ```sql
  CREATE TABLE compilation_build_mode (
      id INTEGER PRIMARY KEY,
      mode INTEGER,
      FOREIGN KEY (id) REFERENCES compilations(id)
  );
  ```
- **Build Mode**

  ```
  case @compilation_build_mode.mode of
    0 = @build_mode_none
  | 1 = @build_mode_manual
  | 2 = @build_mode_auto
  ```

### 4. **compilation_compiling_files**
- **描述**: 记录编译器调用中编译的源文件。
- **属性**:
  - `id` (INTEGER, 外键): 编译器调用的ID，引用自`compilations`表。
  - `num` (INTEGER): 文件的序号。
  - `file` (INTEGER, 外键): 编译的文件的ID，引用自`files`表。
- **主键**: (`id`, `num`)
- **示例**:

  ```sql
  CREATE TABLE compilation_compiling_files (
      id INTEGER,
      num INTEGER,
      file INTEGER,
      PRIMARY KEY (id, num),
      FOREIGN KEY (id) REFERENCES compilations(id),
      FOREIGN KEY (file) REFERENCES files(id)
  );
  ```

  For command `$gcc -c f1.c f2.c f3.c`

  ```
  | num | arg  |
  | --- | ---- |
  | 0   | f1.c |
  | 1   | f2.c |
  | 2   | f3.c |
  ```

  Note: rows only contains the files that were actually compiled, does not include `#include` directives.

### 5. **compilation_time** :DONE
- **描述**: 记录编译器调用中各阶段的耗时。
- **属性**:
- `id` (INTEGER, 外键): 编译器调用的ID，引用自`compilations`表。
- `num` (INTEGER): 文件的序号。
- `kind` (INTEGER): 时间的种类
  ```
  kind:
   1 = frontend_cpu_seconds (CPU seconds used by the extractor frontend)
   2 = frontend_elapsed_seconds (Elapsed seconds during the extractor frontend)
   3 = extractor_cpu_seconds (CPU seconds used by the extractor backend)
   4 = extractor_elapsed_seconds (Elapsed seconds during the extractor backend)
  ```
- `seconds` (FLOAT): 耗时（以秒为单位）。
- **主键**: (`id`, `num`, `kind`)
- **示例**:

```sql
CREATE TABLE compilation_time (
    id INTEGER,
    num INTEGER,
    kind INTEGER,
    seconds FLOAT,
    PRIMARY KEY (id, num, kind),
    FOREIGN KEY (id) REFERENCES compilations(id)
);
```

### 6. **diagnostic_for**
- **描述**: 记录编译器调用中生成的错误或警告信息。

  > The diagnostic message `diagnostic` was generated during compiler invocation `compilation`, and is the `file_number_diagnostic_number`th message generated while extracting the `file_number`th file of that invocation.

- **属性**:
  - `diagnostic` (INTEGER, 主键): 诊断信息的ID，引用自`diagnostics`表。
  - `compilation` (INTEGER, 外键): 编译器调用的ID，引用自`compilations`表。
  - `file_number` (INTEGER): 文件的序号。
  - `file_number_diagnostic_number` (INTEGER): 诊断信息的序号。
- **主键**: (`diagnostic`, `compilation`, `file_number`, `file_number_diagnostic_number`)
- **示例**:
  ```sql
  CREATE TABLE diagnostic_for (
      diagnostic INTEGER,
      compilation INTEGER,
      file_number INTEGER,
      file_number_diagnostic_number INTEGER,
      PRIMARY KEY (diagnostic, compilation, file_number, file_number_diagnostic_number),
      FOREIGN KEY (compilation) REFERENCES compilations(id),
      FOREIGN KEY (diagnostic) REFERENCES diagnostics(id)
  );
  ```

### 7. **compilation_finished** :DONE
- **描述**: 记录编译器调用的完成信息。
- **属性**:
  - `id` (INTEGER, 主键): 编译器调用的ID，引用自`compilations`表。
  - `cpu_seconds` (FLOAT): CPU时间。
  - `elapsed_seconds` (FLOAT): 实际耗时。
- **示例**:
  ```sql
  CREATE TABLE compilation_finished (
      id INTEGER PRIMARY KEY,
      cpu_seconds FLOAT,
      elapsed_seconds FLOAT,
      FOREIGN KEY (id) REFERENCES compilations(id)
  );
  ```

### 8. **externalData**
- **描述**: 记录在快照创建期间从CSV文件加载的外部数据。更多信息请参见[教程：在快照中包含外部数据](https://help.semmle.com/wiki/display/SD/Tutorial%3A+Incorporating+external+data)。
- **属性**:
  - `id` (INTEGER, 主键): 外部数据的唯一标识符，引用自`externalDataElement`。
  - `path` (TEXT): 路径
  - `column` (INTEGER): 列号
  - `value` (TEXT): 值
- **示例**:
  ```sql
  CREATE TABLE externalData (
      id INTEGER PRIMARY KEY,
      path INTEGER,
      column INTEGER,
      value INTEGER,
  );
  ```

### 9. **sourceLocationPrefix**
- **描述**: 快照的源位置前缀。
- **属性**:
  - `prefix` (TEXT, 主键): 前缀
- **示例**:
  ```sql
  CREATE TABLE sourceLocationPrefix (
      prefix INTEGER PRIMARY KEY,
  );
  ```

### 10. **external_packages**
- **描述**: 记录在编译过程中使用的外部包的信息。
- **属性**:
  - `id` (INTEGER, 主键): 包的唯一标识符。
  - `namespace` (TEXT): 包管理器的名称（例如，"dpkg" 或 "yum"）。
  - `package_name` (TEXT): 包的名称。
  - `version` (TEXT): 包的版本。
- **示例**:
  ```sql
  CREATE TABLE external_packages (
      id INTEGER PRIMARY KEY,
      namespace TEXT,
      package_name TEXT,
      version TEXT
  );
  ```

### 11. **header_to_external_package**
- **描述**: 记录文件与外部包的关联。
- **属性**:
  - `fileid` (INTEGER, 外键): 文件的ID，引用自`files`表。
  - `package` (INTEGER, 外键): 包的ID，引用自`external_packages`表。
- **示例**:
  ```sql
  CREATE TABLE header_to_external_package (
      fileid INTEGER,
      package INTEGER,
      FOREIGN KEY (fileid) REFERENCES files(id),
      FOREIGN KEY (package) REFERENCES external_packages(id)
  );
  ```

### 12. **svnentries**
- **描述**: 记录SVN提交的详细信息。
- **属性**:
  - `id` (INTEGER, 主键): 提交的ID。
  - `revision` (TEXT): 提交的修订版本。
  - `author` (TEXT): 作者。
  - `revisionDate` (DATE): 提交日期。
  - `changeSize` (INTEGER): 更改的大小。
- **示例**:
  ```sql
  CREATE TABLE svnentries (
      id INTEGER PRIMARY KEY,
      revision TEXT,
      author TEXT,
      revisionDate DATE,
      changeSize INTEGER
  );
  ```

### 13. **svnaffectedfiles**
- **描述**: 记录SVN提交中受影响的文件。
- **属性**:
  - `id` (INTEGER, 外键): 提交的ID，引用自`svnentries`表。
  - `file` (INTEGER, 外键): 文件的ID，引用自`files`表。
  - `action` (TEXT): 操作类型（例如，添加、删除等）。
- **示例**:
  ```sql
  CREATE TABLE svnaffectedfiles (
      id INTEGER,
      file INTEGER,
      action TEXT,
      FOREIGN KEY (id) REFERENCES svnentries(id),
      FOREIGN KEY (file) REFERENCES files(id)
  );
  ```

### 14. **svnentrymsg**
- **描述**: 记录SVN提交的提交信息。
- **属性**:
  - `id` (INTEGER, 外键): 提交的ID，引用自`svnentries`表。
  - `message` (TEXT): 提交信息。
- **示例**:
  ```sql
  CREATE TABLE svnentrymsg (
      id INTEGER PRIMARY KEY,
      message TEXT,
      FOREIGN KEY (id) REFERENCES svnentries(id)
  );
  ```

### 15. **svnchurn**
- **描述**: 记录SVN提交中每个文件的添加和删除行数。
- **属性**:
  - `commit` (INTEGER, 外键): 提交的ID，引用自`svnentries`表。
  - `file` (INTEGER, 外键): 文件的ID，引用自`files`表。
  - `addedLines` (INTEGER): 添加的行数。
  - `deletedLines` (INTEGER): 删除的行数。
- **示例**:
  ```sql
  CREATE TABLE svnchurn (
      commit INTEGER,
      file INTEGER,
      addedLines INTEGER,
      deletedLines INTEGER,
      FOREIGN KEY (commit) REFERENCES svnentries(id),
      FOREIGN KEY (file) REFERENCES files(id)
  );
  ```

### 16. **extractor_version**
- **描述**: 记录提取器的版本信息。
- **属性**:
  - `codeql_version` (TEXT): CodeQL的版本。
  - `frontend_version` (TEXT): 前端的版本。
- **示例**:
  ```sql
  CREATE TABLE extractor_version (
      codeql_version TEXT,
      frontend_version TEXT
  );
  ```

### 17. **location** +-+ :DONE
```
@location = @location_stmt | @location_expr | @location_default;
```

- **描述**: 记录代码中元素的位置信息。父表
- **属性**:
  - `id` (INTEGER, 主键): 位置的ID。
  - `associated_id` (INTEGER, 外键): 子表id
  - `type` (INTEGER): 子表的类型
    - 1: @location_stmt
    - 2: @location_expr
    - 3: @location_default
- **示例**:

  ```sql
  CREATE TABLE locations (
      id INTEGER PRIMARY KEY,
      associated_id INTEGER,
      type INTEGER,
  );
  ```

### 18. **locations_default** :DONE

- **描述**: 记录代码中元素的位置信息。
- **属性**:
  - `id` (INTEGER, 主键): 位置的ID。
  - `container` (INTEGER, 外键): 容器元素的ID。refer from @container
  - `startLine` (INTEGER): 起始行。
  - `startColumn` (INTEGER): 起始列。
  - `endLine` (INTEGER): 结束行。
  - `endColumn` (INTEGER): 结束列。
- **示例**:

  ```sql
  CREATE TABLE locations_default (
      id INTEGER PRIMARY KEY,
      container INTEGER,
      startLine INTEGER,
      startColumn INTEGER,
      endLine INTEGER,
      endColumn INTEGER,
      FOREIGN KEY (container) REFERENCES container(id)
  );
  ```

### 19. **locations_stmt** :DONE

- **描述**: 记录代码中stmt的位置信息。
- **属性**:
  - `id` (INTEGER, 主键): 位置的ID。
  - `container` (INTEGER, 外键): 容器元素的ID。refer from @container
  - `startLine` (INTEGER): 起始行。
  - `startColumn` (INTEGER): 起始列。
  - `endLine` (INTEGER): 结束行。
  - `endColumn` (INTEGER): 结束列。
- **示例**:

  ```sql
  CREATE TABLE locations_default (
      id INTEGER PRIMARY KEY,
      container INTEGER,
      startLine INTEGER,
      startColumn INTEGER,
      endLine INTEGER,
      endColumn INTEGER,
      FOREIGN KEY (container) REFERENCES container(id)
  );
  ```

### 20. **locations_expr** :DONE

- **描述**: 记录代码中表达式的位置信息。
- **属性**:
  - `id` (INTEGER, 主键): 位置的ID。
  - `container` (INTEGER, 外键): 容器元素的ID。refer from @container
  - `startLine` (INTEGER): 起始行。
  - `startColumn` (INTEGER): 起始列。
  - `endLine` (INTEGER): 结束行。
  - `endColumn` (INTEGER): 结束列。
- **示例**:

  ```sql
  CREATE TABLE locations_default (
      id INTEGER PRIMARY KEY,
      container INTEGER,
      startLine INTEGER,
      startColumn INTEGER,
      endLine INTEGER,
      endColumn INTEGER,
      FOREIGN KEY (container) REFERENCES container(id)
  );
  ```


### 21. **sourceline**
  ```
    @sourceline = @file | @function | @variable | @enumconstant | @xmllocatable; +-+
  ```
- **描述**: 包含行数信息的元素。
- **属性**:
  - `id` (INTEGER, 主键): 主键
  - `associated_id` (INTEGER, 外键): 元素的ID。
  - `type` (INTEGER): 元素的类型。
    - 1: @file
    - 2: @function
    - 3: @variable
    - 4: @enumconstant
    - 5: @xmllocatable
- **示例**:
  ```sql
  CREATE TABLE sourceline (
      id INTEGER PRIMARY KEY,
      associated_id INTEGER,
      type INTEGER,
  )
  ```


### 22. **numlines**
- **描述**: 记录代码中元素的总行数、代码行数、注释行数。
- **属性**:
  - `element_id` (INTEGER, 外键): 元素的ID。
  - `num_lines` (INTEGER): 总行数。
  - `num_code` (INTEGER): 代码行数。
  - `num_comment` (INTEGER): 注释行数。
- **示例**:
  ```sql
  CREATE TABLE numlines (
      element_id INTEGER,
      num_lines INTEGER,
      num_code INTEGER,
      num_comment INTEGER,
      FOREIGN KEY (element_id) REFERENCES sourceline(id)
  );
  ```

### 23. **diagnostics**
- **描述**: 记录诊断信息（错误或警告）。
- **属性**:
  - `id` (INTEGER, 主键): 诊断信息的ID。
  - `severity` (INTEGER): 严重程度。
  - `error_tag` (TEXT): 错误标签。
  - `error_message` (TEXT): 错误信息。
  - `full_error_message` (TEXT): 完整的错误信息。
  - `location` (INTEGER, 外键): 位置ID，引用自`locations_default`表。
- **示例**:
  ```sql
  CREATE TABLE diagnostics (
      id INTEGER PRIMARY KEY,
      severity INTEGER,
      error_tag TEXT,
      error_message TEXT,
      full_error_message TEXT,
      location INTEGER,
      FOREIGN KEY (location) REFERENCES locations_default(id)
  );
  ```

### 24. **files** :DONE
- **描述**: 记录文件信息。
- **属性**:
  - `id` (INTEGER, 主键): 文件的ID。
  - `name` (TEXT): 文件名。
- **示例**:
  ```sql
  CREATE TABLE files (
      id INTEGER PRIMARY KEY,
      name TEXT
  );
  ```

### 25. **folders** :DONE
- **描述**: 记录文件夹信息。
- **属性**:
  - `id` (INTEGER, 主键): 文件夹的ID。
  - `name` (TEXT): 文件夹名。
- **示例**:
  ```sql
  CREATE TABLE folders (
      id INTEGER PRIMARY KEY,
      name TEXT
  );
  ```

### 26. **container** :DONE
```
@container = @folder | @file +-+
```

### 27. **containerparent**
- **描述**: 记录容器元素的父子关系。
- **属性**:
  - `parent` (INTEGER, 外键): 父容器的ID, reference to `container` table
  - `child` (INTEGER, 主键): 子容器的ID, reference to `container` table
- **示例**:
  ```sql
  CREATE TABLE containerparent (
      parent INTEGER,
      child INTEGER PRIMARY KEY,
      FOREIGN KEY (parent) REFERENCES container(id),
      FOREIGN KEY (child) REFERENCES container(id)
  );
  ```

### 28. **fileannotations**
- **描述**: 记录文件的注释信息。
- **属性**:
  - `id` (INTEGER, 外键): 文件的ID，引用自`files`表。
  - `kind` (INTEGER): 注释的类型。
  - `name` (TEXT): 注释的名称。
  - `value` (TEXT): 注释的值。
- **示例**:
  ```sql
  CREATE TABLE fileannotations (
      id INTEGER,
      kind INTEGER,
      name TEXT,
      value TEXT,
      FOREIGN KEY (id) REFERENCES files(id)
  );
  ```

### 29. **inmacroexpansion**
- **描述**: 记录元素在宏展开中的位置。
- **属性**:
  - `id` (INTEGER, 外键): 元素的ID, reference to `element` table
  - `inv` (INTEGER, 外键): 宏调用的ID, reference to `macroinvocation` table
- **示例**:
  ```sql
  CREATE TABLE inmacroexpansion (
      id INTEGER,
      inv INTEGER,
      FOREIGN KEY (id) REFERENCES element(id),
      FOREIGN KEY (inv) REFERENCES macroinvocation(id)
  );
  ```

### 30. **affectedbymacroexpansion**
- **描述**: 记录元素受宏展开影响的程度。
- **属性**:
  - `id` (INTEGER, 外键): 元素的ID, reference to `element` table
  - `inv` (INTEGER, 外键): 宏调用的ID, reference to `macroinvocation` table
- **示例**:
  ```sql
  CREATE TABLE affectedbymacroexpansion (
      id INTEGER,
      inv INTEGER,
      FOREIGN KEY (id) REFERENCES element(id),
      FOREIGN KEY (inv) REFERENCES macroinvocation(id)
  );
  ```

### 31. **macroinvocations**
- **描述**: 记录宏调用的信息。
- **属性**:

  - `id` (INTEGER, 主键): 宏调用的ID。
  - `macro_id` (INTEGER, 外键): 宏定义的ID, reference to `ppd_define` table
  - `location` (INTEGER, 外键): 位置ID，引用自`locations_default`表
  - `kind` (INTEGER): 宏调用的类型。
    ```
    case @macroinvocation.kind of
      1 = @macro_expansion
    | 2 = @other_macro_reference
    ;
    ```

- **示例**:
  ```sql
  CREATE TABLE macroinvocations (
      id INTEGER PRIMARY KEY,
      macro_id INTEGER,
      location INTEGER,
      kind INTEGER,
      FOREIGN KEY (macro_id) REFERENCES ppd_define(id),
      FOREIGN KEY (location) REFERENCES locations_default(id)
  );
  ```

### 32. **macroparent**
- **描述**: 记录宏调用的父子关系。
- **属性**:
  - `id` (INTEGER, 主键): 宏调用的ID
  - `parent_id` (INTEGER, 外键): 父宏调用的ID, reference to `macroinvocation` table
- **示例**:
  ```sql
  CREATE TABLE macroparent (
      id INTEGER PRIMARY KEY,
      parent_id INTEGER,
      FOREIGN KEY (parent_id) REFERENCES macroinvocation(id)
  );
  ```

### 33. **macrolocationbind**
- **描述**: 记录宏调用与位置的关系。
- **属性**:
  - `id` (INTEGER, 外键): 宏调用的ID, reference to `macroinvocation` table
  - `location` (INTEGER, 外键): 位置ID, reference to `locations` table
- **示例**:
  ```sql
  CREATE TABLE macrolocationbind (
      id INTEGER,
      location INTEGER,
      FOREIGN KEY (id) REFERENCES macroinvocation(id),
      FOREIGN KEY (location) REFERENCES location(id)
  );
  ```

### 34. **macro_argument_unexpanded**
- **描述**: 记录未展开的宏参数。
- **属性**:
  - `invocation` (INTEGER, 外键): 宏调用的ID, reference to `macroinvocation` table
  - `argument_index` (INTEGER): 参数的索引。
  - `text` (TEXT): 参数的文本内容。
- **主键**: (`invocation`, `argument_index`)
- **示例**:
  ```sql
  CREATE TABLE macro_argument_unexpanded (
      invocation INTEGER,
      argument_index INTEGER,
      text TEXT,
      PRIMARY KEY (invocation, argument_index),
      FOREIGN KEY (invocation) REFERENCES macroinvocation(id)
  );
  ```

### 35. **macro_argument_expanded**
- **描述**: 记录展开后的宏参数。
- **属性**:
  - `invocation` (INTEGER, 外键): 宏调用的ID, reference to `macroinvocation` table
  - `argument_index` (INTEGER): 参数的索引。
  - `text` (TEXT): 参数的文本内容。
- **主键**: (`invocation`, `argument_index`)
- **示例**:
  ```sql
  CREATE TABLE macro_argument_expanded (
      invocation INTEGER,
      argument_index INTEGER,
      text TEXT,
      PRIMARY KEY (invocation, argument_index),
      FOREIGN KEY (invocation) REFERENCES macroinvocation(id)
  );
  ```

### 36. **functions** :DONE
- **描述**: 记录函数的信息。
- **属性**:
  - `id` (INTEGER, 主键): 函数的ID。
  - `name` (TEXT): 函数名。
  - `kind` (INTEGER): 函数的类型。
    ```
    case @function.kind of
      1 = @normal_function
    | 2 = @constructor
    | 3 = @destructor
    | 4 = @conversion_function
    | 5 = @operator
    | 6 = @builtin_function // GCC built-in functions, e.g. **builtin\_**memcpy_chk
    | 7 = @user_defined_literal
    | 8 = @deduction_guide
    ;
    ```
- **示例**:
  ```sql
  CREATE TABLE functions (
      id INTEGER PRIMARY KEY,
      name TEXT,
      kind INTEGER
  );
  ```

### 37. **function_entry_point** :DONE
- **描述**: 记录函数的入口点。
- **属性**:
  - `id` (INTEGER, 外键): 函数的ID, reference to `functions` table.
  - `entry_point` (INTEGER, 主键): 入口点的ID, reference to `stmts` table
- **示例**:
  ```sql
  CREATE TABLE function_entry_point (
      id INTEGER,
      entry_point INTEGER,
      PRIMARY KEY (id, entry_point),
      FOREIGN KEY (id) REFERENCES functions(id),
      FOREIGN KEY (entry_point) REFERENCES stmts(id)
  );
  ```

### 38. **function_return_type** :DONE
- **描述**: 记录函数的返回类型。
- **属性**:
  - `id` (INTEGER, 外键): 函数的ID, reference to `functions` table.
  - `return_type` (INTEGER, 外键): 返回类型的ID, reference to `type` table.
- **示例**:
  ```sql
  CREATE TABLE function_return_type (
      id INTEGER,
      return_type INTEGER,
      FOREIGN KEY (id) REFERENCES functions(id),
      FOREIGN KEY (return_type) REFERENCES type(id)
  );
  ```

### 39. **coroutine** :DONE
- **描述**: 记录协程的相关信息。

  > If `function` is a coroutine, then this gives the `std::experimental::resumable_traits` instance associated with it, and the variables representing the `handle` and `promise` for it.

- **属性**:
  - `function` (INTEGER, 主键): 函数的ID, reference to `functions` table.
  - `traits` (INTEGER): 协程特征的ID, reference to `type` table.
- **示例**:
  ```sql
  CREATE TABLE coroutine (
      function INTEGER PRIMARY KEY,
      traits INTEGER,
      FOREIGN KEY (function) REFERENCES functions(id)
      FOREIGN KEY (traits) REFERENCES type(id)
  );
  ```

### 40. **coroutine_placeholder_variable** :TODO
- **描述**: 记录协程占位符变量的信息。
- **属性**:
  - `placeholder_variable` (INTEGER, 主键): 占位符变量的ID, reference to `variable` table.
  - `kind` (INTEGER): 占位符变量的类型。
    ```
    case @coroutine_placeholder_variable.kind of
    1 = @handle
    | 2 = @promise
    | 3 = @init_await_resume
    ;
    ```
  - `function` (INTEGER, 外键): 函数的ID, reference to `functions` table.
- **示例**:
  ```sql
  CREATE TABLE coroutine_placeholder_variable (
      placeholder_variable INTEGER PRIMARY KEY,
      kind INTEGER,
      function INTEGER,
      FOREIGN KEY (placeholder_variable) REFERENCES variable(id)
      FOREIGN KEY (function) REFERENCES functions(id)
  );
  ```

### 41. **coroutine_new** :DONE :FIXME
- **描述**: 记录协程的`new`函数。
- **属性**:
  - `function` (INTEGER, 主键): 函数的ID, reference to `functions` table.
  - `new` (INTEGER): `new`函数的ID, reference to `functions` table.
- **示例**:
  ```sql
  CREATE TABLE coroutine_new (
      function INTEGER PRIMARY KEY,
      new INTEGER,
      FOREIGN KEY (function) REFERENCES functions(id)
      FOREIGN KEY (new) REFERENCES functions(id)
  );
  ```

### 42. **coroutine_delete** :DONE :FIXME
- **描述**: 记录协程的`delete`函数。
- **属性**:
  - `function` (INTEGER, 主键): 函数的ID, reference to `functions` table.
  - `delete` (INTEGER): `delete`函数的ID, reference to `functions` table.
- **示例**:
  ```sql
  CREATE TABLE coroutine_delete (
      function INTEGER PRIMARY KEY,
      delete INTEGER,
      FOREIGN KEY (function) REFERENCES functions(id),
      FOREIGN KEY (delete) REFERENCES functions(id)
  );
  ```

### 43. **purefunctions** :DONE
- **描述**: 记录纯函数的信息。
- **属性**:
  - `id` (INTEGER, 主键): 函数的ID, reference to `functions` table.
- **示例**:
  ```sql
  CREATE TABLE purefunctions (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES functions(id)
  );
  ```

### 44. **function_deleted** :DONE
- **描述**: 记录被删除的函数的信息。
- **属性**:
  - `id` (INTEGER, 主键): 函数的ID, reference to `functions` table.
- **示例**:
  ```sql
  CREATE TABLE function_deleted (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES functions(id)
  );
  ```

### 45. **function_defaulted** :DONE
- **描述**: 记录默认函数的详细信息。
- **属性**:
  - `id` (INTEGER, 主键): 函数的ID, reference to `functions` table.
- **示例**:
  ```sql
  CREATE TABLE function_defaulted (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES functions(id)
  );
  ```

### 46. **function_prototyped** :DONE
- **描述**: 记录函数原型。
- **属性**:
  - `id` (INTEGER, 主键): 函数的ID, reference to `functions` table.
- **示例**:
  ```sql
  CREATE TABLE function_prototyped (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES functions(id)
  );
  ```

### 47. **deduction_guide_for_class** :DONE
- **描述**: 记录类的推导指南。
- **属性**:
  - `id` (INTEGER, 外键): 函数的ID, reference to `functions` table.
  - `class_template` (INTEGER, 外键): 类模板的ID, reference to `usertype` table.
- **示例**:
  ```sql
  CREATE TABLE deduction_guide_for_class (
      id INTEGER,
      class_template INTEGER,
      FOREIGN KEY (id) REFERENCES functions(id),
      FOREIGN KEY (class_template) REFERENCES usertype(id)
  );
  ```

### 48. **member_function_this_type** :TODO
- **描述**: 记录成员函数的`this`类型。
- **属性**:
  - `id` (INTEGER, 主键): 函数的ID, reference to `functions` table.
  - `this_type` (INTEGER, 外键): `this`类型的ID, reference to `type` table.
- **示例**:
  ```sql
  CREATE TABLE member_function_this_type (
      id INTEGER PRIMARY KEY,
      this_type INTEGER,
      FOREIGN KEY (id) REFERENCES functions(id),
      FOREIGN KEY (this_type) REFERENCES type(id)
  );
  ```

### 49. **fun_decls** :DONE
- **描述**: 记录函数声明的信息。
- **属性**:
  - `id` (INTEGER, 主键): 函数声明的ID
  - `function` (INTEGER, 外键): 函数的ID, reference to `function` table.
  - `type_id` (INTEGER, 外键): 类型的ID, reference to `type` table.
  - `name` (TEXT): 函数名。
  - `location` (INTEGER, 外键): 位置ID，引用自`location_default`表。
- **示例**:
  ```sql
  CREATE TABLE fun_decls (
      id INTEGER PRIMARY KEY,
      function INTEGER,
      type_id INTEGER,
      name TEXT,
      location INTEGER,
      PRIMARY KEY (id, type_id),
      FOREIGN KEY (function) REFERENCES function(id),
      FOREIGN KEY (type_id) REFERENCES type(id),
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 50. **fun_def** :DONE
- **描述**: 记录函数定义的详细信息。
- **属性**:
  - `id` (INTEGER, 主键): 函数声明的ID, reference to `fun_decls` table.
- **示例**:
  ```sql
  CREATE TABLE fun_def (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES fun_decls(id)
  );
  ```

### 51. **fun_specialized** :DONE
- **描述**: 记录函数特化的信息。
- **属性**:
  - `id` (INTEGER, 主键): 函数声明的ID, reference to `fun_decls` table.
- **示例**:
  ```sql
  CREATE TABLE fun_specialized (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES fun_decls(id)
  );
  ```

### 52. **fun_implicit** :DONE
- **描述**: 记录隐式函数的详细信息。
- **属性**:
  - `id` (INTEGER, 主键): 函数声明的ID, reference to `fun_decls` table.
- **示例**:
  ```sql
  CREATE TABLE fun_implicit (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES fun_decls(id)
  );
  ```

### 53. **fun_decl_specifiers** :TODO
- **描述**: 记录函数声明的说明符。
- **属性**:
  - `id` (INTEGER, 外键): 函数声明的ID, reference to `fun_decls` table.
  - `name` (TEXT): 说明符的名称。
- **示例**:
  ```sql
  CREATE TABLE fun_decl_specifiers (
      id INTEGER,
      name TEXT,
      FOREIGN KEY (id) REFERENCES fun_decls(id)
  );
  ```

### 54. **fun_decl_throws** :DONE
- **描述**: 记录函数声明的抛出说明。
- **属性**:
  - `fun_decl` (INTEGER, 外键): 函数声明的ID, reference to `fun_decls` table.
  - `index` (INTEGER): 索引。
  - `type_id` (INTEGER, 外键): 类型的ID, reference to `type` table.
- **示例**:
  ```sql
  CREATE TABLE fun_decl_throws (
      fun_decl INTEGER,
      index INTEGER,
      type_id INTEGER,
      PRIMARY KEY (fun_decl, index)
      FOREIGN KEY (fun_decl) REFERENCES fun_decls(id),
      FOREIGN KEY (type_id) REFERENCES type(id)
  );
  ```

### 55. **fun_decl_empty_throws** :DONE
- **描述**: 记录函数声明的空抛出说明。
- **属性**:
  - `fun_decl` (INTEGER, 主键): 函数声明的ID, reference to `fun_decls` table.
- **示例**:
  ```sql
  CREATE TABLE fun_decl_empty_throws (
      fun_decl INTEGER PRIMARY KEY,
      FOREIGN KEY (fun_decl) REFERENCES fun_decls(id)
  );
  ```

### 56. **fun_decl_noexcept** :DONE
- **描述**: 记录函数声明的`noexcept`说明。
- **属性**:
  - `fun_decl` (INTEGER, 外键): 函数声明的ID, reference to `fun_decls` table.
  - `constant` (INTEGER, 外键): 常量的ID, reference to `expr` table.
- **示例**:
  ```sql
  CREATE TABLE fun_decl_noexcept (
      fun_decl INTEGER,
      constant INTEGER,
      FOREIGN KEY (fun_decl) REFERENCES fun_decls(id),
      FOREIGN KEY (constant) REFERENCES expr(id)
  );
  ```

### 57. **fun_decl_empty_noexcept** :DONE
- **描述**: 记录函数声明的空`noexcept`说明。
- **属性**:
  - `fun_decl` (INTEGER, 主键): 函数声明的ID, reference to `fun_decls` table.
- **示例**:
  ```sql
  CREATE TABLE fun_decl_empty_noexcept (
      fun_decl INTEGER PRIMARY KEY,
      FOREIGN KEY (fun_decl) REFERENCES fun_decls(id)
  );
  ```

### 58. **fun_decl_typedef_type** :DONE
- **描述**: 记录函数声明的`typedef`类型。
- **属性**:
  - `fun_decl` (INTEGER, 主键): 函数声明的ID, reference to `fun_decls` table.
  - `typedeftype_id` (INTEGER, 外键): `typedef`类型的ID, reference to `usertype` table.
- **示例**:
  ```sql
  CREATE TABLE fun_decl_typedef_type (
      fun_decl INTEGER PRIMARY KEY,
      typedeftype_id INTEGER,
      FOREIGN KEY (fun_decl) REFERENCES fun_decls(id),
      FOREIGN KEY (typedeftype_id) REFERENCES usertype(id)
  );
  ```

### 59. **fun_requires** :TODO
- **描述**: 记录函数声明的`requires`说明。
- **属性**:
  - `id` (INTEGER, 主键): 函数声明的ID, reference to `fun_decls` table.
  - `kind` (INTEGER): 类型的种类。
    ```
    case @fun_requires.kind of
      1 = @template_attached
    | 2 = @function_attached
    ;
    ```
  - `constraint` (INTEGER, 外键): 约束条件的ID, reference to `expr` table.
- **示例**:
  ```sql
  CREATE TABLE fun_requires (
      id INTEGER PRIMARY KEY,
      kind INTEGER,
      constraint INTEGER,
      FOREIGN KEY (id) REFERENCES fun_decls(id),
      FOREIGN KEY (constraint) REFERENCES expr(id)
  );
  ```

### 60. **param_decl_bind** :TODO
- **描述**: 记录参数声明的绑定。
- **属性**:
  - `id` (INTEGER, 主键): 参数声明的ID, reference to `var_decls` table.
  - `index` (INTEGER): 参数的索引。
  - `fun_decl` (INTEGER, 外键): 函数声明的ID, reference to `fun_decls` table.
- **示例**:
  ```sql
  CREATE TABLE param_decl_bind (
      id INTEGER PRIMARY KEY,
      index INTEGER,
      fun_decl INTEGER,
      FOREIGN KEY (id) REFERENCES var_decls(id),
      FOREIGN KEY (fun_decl) REFERENCES fun_decls(id)
  );
  ```

### 61. **var_decls** :DONE
- **描述**: 记录变量声明的信息。
- **属性**:
  - `id` (INTEGER, 主键): 变量声明的ID
  - `variable` (INTEGER, 外键): 变量的ID, reference to `variable` table.
  - `type_id` (INTEGER, 外键): 类型的ID, reference to `type` table.
  - `name` (TEXT): 变量名。
  - `location` (INTEGER, 外键): 位置ID，引用自`location_default`表。
- **示例**:
  ```sql
  CREATE TABLE var_decls (
      id INTEGER PRIMARY KEY,
      variable INTEGER,
      type_id INTEGER,
      name TEXT,
      location INTEGER,
      PRIMARY KEY (id, type_id),
      FOREIGN KEY (variable) REFERENCES variable(id),
      FOREIGN KEY (type_id) REFERENCES type(id),
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 62. **var_def** :DONE
- **描述**: 记录变量定义的详细信息。
- **属性**:
  - `id` (INTEGER, 主键): 变量声明的ID, reference to `var_decls` table.
- **示例**:
  ```sql
  CREATE TABLE var_def (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES var_decls(id)
  );
  ```

### 63. **var_specialized** :DONE
- **描述**: 记录变量特化的信息。
- **属性**:
  - `id` (INTEGER, 外键): 变量声明的ID, reference to `var_decls` table.
- **示例**:
  ```sql
  CREATE TABLE var_specialized (
      id INTEGER,
      FOREIGN KEY (id) REFERENCES var_decls(id)
  );
  ```

### 64. **var_decl_specifiers** :DONE
- **描述**: 记录变量声明的说明符。
- **属性**:
  - `id` (INTEGER, 外键): 变量声明的ID, reference to `var_decls` table.
  - `name` (TEXT): 说明符的名称。
- **示例**:
  ```sql
  CREATE TABLE var_decl_specifiers (
      id INTEGER,
      name TEXT,
      FOREIGN KEY (id) REFERENCES var_decls(id)
  );
  ```

### 65. **is_structured_binding** :DONE
- **描述**: 记录变量是否为结构化绑定
- **属性**:
  - `id` (INTEGER, 主键): 变量的ID, reference to `variable` table.
- **示例**:
  ```sql
  CREATE TABLE is_structured_binding (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES variable(id)
  );
  ```

### 66. **var_requires** :DONE
- **描述**: 记录变量声明的`requires`说明。
- **属性**:
  - `id` (INTEGER, 外键): 变量声明的ID, reference to `var_decls` table.
  - `constraint` (INTEGER, 外键): 约束条件的ID, reference to `expr` table.
- **示例**:
  ```sql
  CREATE TABLE var_requires (
      id INTEGER,
      constraint INTEGER,
      FOREIGN KEY (id) REFERENCES var_decls(id),
      FOREIGN KEY (constraint) REFERENCES expr(id)
  );
  ```

### 67. **type_decls** :DONE
- **描述**: 记录类型声明的信息。
- **属性**:
  - `id` (INTEGER, 主键): 类型声明的ID。
  - `type_id` (INTEGER, 外键): 类型的ID, reference to `type` table.
  - `location` (INTEGER, 外键): 位置ID，引用自`location_default`表。
- **示例**:
  ```sql
  CREATE TABLE type_decls (
      id INTEGER PRIMARY KEY,
      type_id INTEGER,
      location INTEGER,
      FOREIGN KEY (type_id) REFERENCES type(id),
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 68. **type_def** :DONE
- **描述**: 记录类型定义的详细信息。
- **属性**:
  - `id` (INTEGER, 主键): 类型声明的ID, reference to `type_decls` table.
- **示例**:
  ```sql
  CREATE TABLE type_def (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES type_decls(id)
  );
  ```

### 69. **type_decl_top** :DONE
- **描述**: 记录顶级类型声明。
- **属性**:
  - `type_decl` (INTEGER, 主键): 类型声明的ID, reference to `type_decls` table.
- **示例**:
  ```sql
  CREATE TABLE type_decl_top (
      type_decl INTEGER PRIMARY KEY,
      FOREIGN KEY (type_decl) REFERENCES type_decls(id)
  );
  ```

### 70. **type_requires**
- **描述**: 记录类型声明的`requires`说明。
- **属性**:
  - `id` (INTEGER, 外键): 类型声明的ID, reference to `type_decls` table.
  - `constraint` (INTEGER, 外键): 约束条件的ID, reference to `expr` table.
- **示例**:
  ```sql
  CREATE TABLE type_requires (
      id INTEGER,
      constraint INTEGER,
      FOREIGN KEY (id) REFERENCES type_decls(id),
      FOREIGN KEY (constraint) REFERENCES expr(id)
  );
  ```

### 71. **namespace_decls**
- **描述**: 记录命名空间声明的信息。
- **属性**:
  - `id` (INTEGER, 主键): 命名空间声明的ID。
  - `namespace_id` (INTEGER, 外键): 命名空间的ID, reference to `namespace` table.
  - `location` (INTEGER, 外键): 位置ID，引用自`location_default`表。
  - `bodylocation` (INTEGER, 外键): 体位置ID, reference to `location_default` table.
- **示例**:
  ```sql
  CREATE TABLE namespace_decls (
      id INTEGER PRIMARY KEY,
      namespace_id INTEGER,
      location INTEGER,
      bodylocation INTEGER,
      FOREIGN KEY (namespace_id) REFERENCES namespace(id),
      FOREIGN KEY (location) REFERENCES location_default(id),
      FOREIGN KEY (bodylocation) REFERENCES location_default(id)
  );
  ```

### 72. **usings**
- **描述**: 记录`using`声明的信息。
- **属性**:
  - `id` (INTEGER, 主键): `using`声明的ID。
  - `element_id` (INTEGER, 外键): 元素的ID, reference to `element` table.
  - `location` (INTEGER, 外键): 位置ID，引用自`location_default`表。
  - `kind` (INTEGER): `using`声明的种类。
    ```
    case @using.kind of
      1 = @using_declaration
    | 2 = @using_directive
    | 3 = @using_enum_declaration
    ;
    ```
- **示例**:
  ```sql
  CREATE TABLE usings (
      id INTEGER PRIMARY KEY,
      element_id INTEGER,
      location INTEGER,
      kind INTEGER,
      FOREIGN KEY (element_id) REFERENCES element(id),
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 73. **using_container**
- **描述**: 包含`using`声明的元素。
- **属性**:
  - `parent` (INTEGER, 外键): 父元素的ID，引用自`element`表。
  - `child` (INTEGER, 主键): `using`声明的ID，引用自`using`表。
- **示例**:
  ```sql
  CREATE TABLE using_container (
      parent INTEGER,
      child INTEGER PRIMARY KEY,
      FOREIGN KEY (parent) REFERENCES element(id),
      FOREIGN KEY (child) REFERENCES using(id)
  );
  ```

### 74. **static_asserts**
- **描述**: 记录静态断言的信息。
- **属性**:
  - `id` (INTEGER, 主键): 静态断言的ID。
  - `condition` (INTEGER, 外键): 条件的ID, reference to `expr` table.
  - `message` (TEXT): 消息。
  - `location` (INTEGER, 外键): 位置ID，引用自`location_default`表。
  - `enclosing` (INTEGER, 外键): 包含元素的ID, reference to `element` table.
- **示例**:
  ```sql
  CREATE TABLE static_asserts (
      id INTEGER PRIMARY KEY,
      condition INTEGER,
      message TEXT,
      location INTEGER,
      enclosing INTEGER,
      FOREIGN KEY (condition) REFERENCES expr(id),
      FOREIGN KEY (location) REFERENCES location_default(id),
      FOREIGN KEY (enclosing) REFERENCES element(id)
  );
  ```

### 75. **params** :DONE
- **描述**: 记录函数参数的信息。
- **属性**:
  - `id` (INTEGER, 主键): 参数的ID。@parameter
  - `function` (INTEGER, 外键): 函数的ID, reference to `parameterized_element` table.
  - `index` (INTEGER): 参数的索引。
  - `type_id` (INTEGER, 外键): 类型的ID, reference to `type` table.
- **示例**:
  ```sql
  CREATE TABLE params (
      id INTEGER PRIMARY KEY,
      function INTEGER,
      index INTEGER,
      type_id INTEGER,
      PRIMARY KEY (id, type_id),
      PRIMARY KEY (function, index, type_id)
      FOREIGN KEY (function) REFERENCES parameterized_element(id),
      FOREIGN KEY (type_id) REFERENCES type(id)
  );
  ```

### 76. **overrides**
- **描述**: 记录函数重写的信息。
- **属性**:
  - `new` (INTEGER, 外键): 新函数的ID, reference to `function` table.
  - `old` (INTEGER, 外键): 旧函数的ID, reference to `function` table.
- **示例**:
  ```sql
  CREATE TABLE overrides (
      new INTEGER,
      old INTEGER,
      FOREIGN KEY (new) REFERENCES function(id),
      FOREIGN KEY (old) REFERENCES function(id)
  );
  ```

### 77. **membervariables** :DONE
- **描述**: 记录成员变量的信息。
- **属性**:
  - `id` (INTEGER, 主键): 成员变量的ID。
  - `type_id` (INTEGER, 外键): 类型的ID, reference to `type` table.
  - `name` (TEXT): 成员变量名。
- **示例**:
  ```sql
  CREATE TABLE membervariables (
      id INTEGER PRIMARY KEY,
      type_id INTEGER,
      name TEXT,
      FOREIGN KEY (type_id) REFERENCES type(id)
  );
  ```

### 78. **globalvariables** :DONE
- **描述**: 记录全局变量的信息。
- **属性**:
  - `id` (INTEGER, 主键): 全局变量的ID。
  - `type_id` (INTEGER, 外键): 类型的ID, reference to `type` table.
  - `name` (TEXT): 全局变量名。
- **示例**:
  ```sql
  CREATE TABLE globalvariables (
      id INTEGER PRIMARY KEY,
      type_id INTEGER,
      name TEXT,
      FOREIGN KEY (type_id) REFERENCES type(id)
  );
  ```

### 79. **localvariables** :DONE
- **描述**: 记录局部变量的信息。
- **属性**:
  - `id` (INTEGER, 主键): 局部变量的ID。
  - `type_id` (INTEGER, 外键): 类型的ID, reference to `type` table.
  - `name` (TEXT): 局部变量名。
- **示例**:
  ```sql
  CREATE TABLE localvariables (
      id INTEGER PRIMARY KEY,
      type_id INTEGER,
      name TEXT,
      FOREIGN KEY (type_id) REFERENCES type(id)
  );
  ```

### 80. **autoderivation**
- **描述**: 记录变量的自动推导信息。
- **属性**:
  - `var` (INTEGER, 主键): 变量的ID。
  - `derivation_type` (INTEGER, 外键): 推导类型的ID, reference to `type` table.
- **示例**:
  ```sql
  CREATE TABLE autoderivation (
      var INTEGER PRIMARY KEY,
      derivation_type INTEGER,
      FOREIGN KEY (derivation_type) REFERENCES type(id)
  );
  ```

### 81. **orphaned_variables** :TODO :?
- **描述**: 记录孤立变量的信息。
- **属性**:
  - `var` (INTEGER, 外键): 局部变量的ID, reference to `localvariable` table.
  - `function` (INTEGER, 外键): 函数的ID, reference to `function` table.
- **示例**:
  ```sql
  CREATE TABLE orphaned_variables (
      var INTEGER,
      function INTEGER,
      FOREIGN KEY (var) REFERENCES localvariable(id),
      FOREIGN KEY (function) REFERENCES function(id)
  );
  ```

### 82. **enumconstants**
- **描述**: 记录枚举常量的信息。
- **属性**:
  - `id` (INTEGER, 主键): 枚举常量的ID。
  - `parent` (INTEGER, 外键): 父类型的ID, reference to `usertype` table.
  - `index` (INTEGER): 索引。
  - `type_id` (INTEGER, 外键): 类型的ID, reference to `type` table.
  - `name` (TEXT): 枚举常量名。
  - `location` (INTEGER, 外键): 位置ID，引用自`location_default`表。
- **示例**:

  ```sql
  CREATE TABLE enumconstants (
      id INTEGER PRIMARY KEY,
      parent INTEGER,
      index INTEGER,
      type_id INTEGER,
      name TEXT,
      location INTEGER,
      FOREIGN KEY (parent) REFERENCES usertype(id),
      FOREIGN KEY (type_id) REFERENCES type(id),
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 83. **variable** :DONE
```
@variable = @localscopevariable | @globalvariable | @membervariable; +-+
```
- **描述**: 变量
- **属性**:

  - `id` (INTEGER PRIMARY KEY): 主键
  - `associate_id` (INTEGER): 子表中的id
  - `type` (INTEGER): 子表的类型
    - 1: @loacalscopevariable
    - 2: @globalvariable
    - 3: @membervariable

- **示例**:
  ```sql
  CREATE TBALE variable (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER
  )
  ```


### 84. **localscopevariable** :DONE
```
@localscopevariable = @localvariable | @parameter; +-+
```
- **描述**: 局部变量
- **属性**:
  - `id` (INTEGER): 主键
  - `associate_id` (INTEGER): 关联的变量ID
  - `type` (INTEGER): 类型
    - 1: @localscopevariable
    - 2: @parameter

- **示例**:
  ```sql
  CREATE TBALE localscopevariable (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER
  )
  ```


### 85. **builtintypes** :DONE
- **描述**: 记录内置类型的详细信息。
- **属性**:

  - `id` (INTEGER, 主键): 内置类型的ID。
  - `name` (TEXT): 内置类型名。
  - `kind` (INTEGER): 内置类型的种类。

    ```
    case @builtintype.kind of
       1 = @errortype
    |  2 = @unknowntype
    |  3 = @void
    |  4 = @boolean
    |  5 = @char
    |  6 = @unsigned_char
    |  7 = @signed_char
    |  8 = @short
    |  9 = @unsigned_short
    | 10 = @signed_short
    | 11 = @int
    | 12 = @unsigned_int
    | 13 = @signed_int
    | 14 = @long
    | 15 = @unsigned_long
    | 16 = @signed_long
    | 17 = @long_long
    | 18 = @unsigned_long_long
    | 19 = @signed_long_long
    // ... 20 Microsoft-specific __int8
    // ... 21 Microsoft-specific __int16
    // ... 22 Microsoft-specific __int32
    // ... 23 Microsoft-specific __int64
    | 24 = @float
    | 25 = @double
    | 26 = @long_double
    | 27 = @complex_float         // C99-specific _Complex float
    | 28 = @complex_double        // C99-specific _Complex double
    | 29 = @complex_long_double   // C99-specific _Complex long double
    | 30 = @imaginary_float       // C99-specific _Imaginary float
    | 31 = @imaginary_double      // C99-specific _Imaginary double
    | 32 = @imaginary_long_double // C99-specific _Imaginary long double
    | 33 = @wchar_t               // Microsoft-specific
    | 34 = @decltype_nullptr      // C++11
    | 35 = @int128                // __int128
    | 36 = @unsigned_int128       // unsigned __int128
    | 37 = @signed_int128         // signed __int128
    | 38 = @float128              // __float128
    | 39 = @complex_float128      // _Complex __float128
    | 40 = @decimal32             // _Decimal32
    | 41 = @decimal64             // _Decimal64
    | 42 = @decimal128            // _Decimal128
    | 43 = @char16_t
    | 44 = @char32_t
    | 45 = @std_float32           // _Float32
    | 46 = @float32x              // _Float32x
    | 47 = @std_float64           // _Float64
    | 48 = @float64x              // _Float64x
    | 49 = @std_float128          // _Float128
    // ... 50 _Float128x
    | 51 = @char8_t
    | 52 = @float16               // _Float16
    | 53 = @complex_float16       // _Complex _Float16
    | 54 = @fp16                  // __fp16
    | 55 = @std_bfloat16          // __bf16
    | 56 = @std_float16           // std::float16_t
    | 57 = @complex_std_float32   // _Complex _Float32
    | 58 = @complex_float32x      // _Complex _Float32x
    | 59 = @complex_std_float64   // _Complex _Float64
    | 60 = @complex_float64x      // _Complex _Float64x
    | 61 = @complex_std_float128  // _Complex _Float128
    ;
    ```

  - `size` (INTEGER): 大小。
  - `sign` (INTEGER): 符号。
  - `alignment` (INTEGER): 对齐。

- **示例**:
  ```sql
  CREATE TABLE builtintypes (
      id INTEGER PRIMARY KEY,
      name TEXT,
      kind INTEGER,
      size INTEGER,
      sign INTEGER,
      alignment INTEGER
  );


### 86. **derivedtypes** :DONE
- **描述**: 记录派生类型的详细信息。
- **属性**:
  - `id` (INTEGER, 主键): 派生类型的ID。
  - `name` (TEXT): 派生类型名。
  - `kind` (INTEGER): 派生类型的种类。
  ```
  case @derivedtype.kind of
     1 = @pointer
  |  2 = @reference
  |  3 = @type_with_specifiers
  |  4 = @array
  |  5 = @gnu_vector
  |  6 = @routineptr
  |  7 = @routinereference
  |  8 = @rvalue_reference // C++11
  // ... 9 type_conforming_to_protocols deprecated
  | 10 = @block
  ;
  ```
  - `type_id` (INTEGER, 外键): 类型的ID, reference to `type` table.
- **示例**:
  ```sql
  CREATE TABLE derivedtypes (
      id INTEGER PRIMARY KEY,
      name TEXT,
      kind INTEGER,
      type_id INTEGER,
      for_id INTEGER,
      FOREIGN KEY (type_id) REFERENCES type(id),
  );
  ```

### 87. **pointerishsize**
- **描述**: 指针类型的大小和对齐信息。
- **属性**:
  - `id` (INTEGER, 主键): 派生类型的唯一标识符，引用自`derivedtype`表。
  - `size` (INTEGER): 大小。
  - `alignment` (INTEGER): 对齐。
- **示例**:
  ```sql
  CREATE TABLE pointerishsize (
      id INTEGER PRIMARY KEY,
      size INTEGER,
      alignment INTEGER,
      FOREIGN KEY (id) REFERENCES derivedtype(id)
  );
  ```

### 88. **arraysizes**
- **描述**: 数组类型的大小和对齐信息。
- **属性**:
  - `id` (INTEGER, 主键): 派生类型的唯一标识符，引用自`derivedtype`表。
  - `num_elements` (INTEGER): 元素数量。
  - `bytesize` (INTEGER): 字节大小。
  - `alignment` (INTEGER): 对齐。
- **示例**:
  ```sql
  CREATE TABLE arraysizes (
      id INTEGER PRIMARY KEY,
      num_elements INTEGER,
      bytesize INTEGER,
      alignment INTEGER,
      FOREIGN KEY (id) REFERENCES derivedtype(id)
  );
  ```

### 89. **typedefbase**
- **描述**: `typedef`的基础类型。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
  - `type_id` (INTEGER, 外键): 类型的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE typedefbase (
      id INTEGER PRIMARY KEY,
      type_id INTEGER,
      FOREIGN KEY (id) REFERENCES usertype(id),
      FOREIGN KEY (type_id) REFERENCES type(id)
  );
  ```

### 90. **decltypes** :DONE
- **描述**: `decltype`运算符的实例。
- **属性**:
  - `id` (INTEGER, 主键): `decltype`的ID，引用自`decltype`。
  - `expr` (INTEGER, 外键): 表达式的ID，引用自`expr`表。
  - `base_type` (INTEGER, 外键): 基础类型的ID，引用自`type`表。
  - `parentheses_would_change_meaning` (BOOLEAN): 括号是否会改变语义。 :?
- **示例**:
  ```sql
  CREATE TABLE decltypes (
      id INTEGER PRIMARY KEY,
      expr INTEGER,
      base_type INTEGER,
      parentheses_would_change_meaning BOOLEAN,
      PRIMARY KEY (id, expr),
      FOREIGN KEY (expr) REFERENCES expr(id),
      FOREIGN KEY (base_type) REFERENCES type(id)
  );
  ```

### 91. **usertypes** :DONE
- **描述**: 用户定义的类型。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符。
  - `name` (TEXT): 类型名称。
  - `kind` (INTEGER): 类型的种类
    ```
    case @usertype.kind of
    |  0 = @unknown_usertype
    |  1 = @struct
    |  2 = @class
    |  3 = @union
    |  4 = @enum
    |  5 = @typedef                       // classic C: typedef typedef type name
    // ... 6 = @template deprecated
    |  7 = @template_parameter
    |  8 = @template_template_parameter
    |  9 = @proxy_class                   // a proxy class associated with a template parameter
    // ... 10 objc_class deprecated
    // ... 11 objc_protocol deprecated
    // ... 12 objc_category deprecated
    | 13 = @scoped_enum
    | 14 = @using_alias                  // a using name = type style typedef
    | 15 = @template_struct
    | 16 = @template_class
    | 17 = @template_union
    ;
    ```
- **示例**:
  ```sql
  CREATE TABLE usertypes (
      id INTEGER PRIMARY KEY,
      name TEXT,
      kind INTEGER
  );
  ```

### 92. **usertypesize**
- **描述**: 用户定义类型的大小和对齐信息。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
  - `size` (INTEGER): 大小。
  - `alignment` (INTEGER): 对齐。
- **示例**:
  ```sql
  CREATE TABLE usertypesize (
      id INTEGER PRIMARY KEY,
      size INTEGER,
      alignment INTEGER,
      FOREIGN KEY (id) REFERENCES usertype(id)
  );
  ```

### 93. **usertype_final**
- **描述**: 用户定义类型的最终状态。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
- **示例**:
  ```sql
  CREATE TABLE usertype_final (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES usertype(id)
  );
  ```

### 94. **usertype_uuid**
- **描述**: 用户定义类型的UUID。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
  - `uuid` (TEXT): UUID。
- **示例**:
  ```sql
  CREATE TABLE usertype_uuid (
      id INTEGER PRIMARY KEY,
      uuid TEXT,
      FOREIGN KEY (id) REFERENCES usertype(id)
  );
  ```

### 95. **nontype_template_parameters**
- **描述**: 非类型模板参数。
- **属性**:
  - `id` (INTEGER, 主键): 表达式的唯一标识符，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE nontype_template_parameters (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES expr(id)
  );
  ```

### 96. **type_template_type_constraint**
- **描述**: 类型模板的类型约束。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
  - `constraint` (INTEGER, 外键): 约束条件的ID，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE type_template_type_constraint (
      id INTEGER PRIMARY KEY,
      constraint INTEGER,
      FOREIGN KEY (id) REFERENCES usertype(id),
      FOREIGN KEY (constraint) REFERENCES expr(id)
  );
  ```

### 97. **mangled_name**
- **描述**: 名称的修饰形式。
- **属性**:
  - `id` (INTEGER, 主键): 声明的唯一标识符，引用自`declaration`表。
  - `mangled_name` (INTEGER, 外键): 修饰名称，引用自`@mangledname`表。
  - `is_complete` (BOOLEAN): 是否完整。
- **示例**:
  ```sql
  CREATE TABLE mangled_name (
      id INTEGER PRIMARY KEY,
      mangled_name INTEGER,
      is_complete BOOLEAN,
      FOREIGN KEY (id) REFERENCES declaration(id),
      FOREIGN KEY (mangled_name) REFERENCES mangledname(id)
  );
  ```

### 98. **is_pod_class** :DONE
- **描述**: 是否为POD类。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
- **示例**:
  ```sql
  CREATE TABLE is_pod_class (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES usertype(id)
  );
  ```

### 99. **is_standard_layout_class** :DONE
- **描述**: 是否为标准布局类。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
- **示例**:
  ```sql
  CREATE TABLE is_standard_layout_class (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES usertype(id)
  );
  ```

### 100. **is_complete** :DONE
- **描述**: 是否完整。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
- **示例**:
  ```sql
  CREATE TABLE is_complete (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES usertype(id)
  );
  ```

### 101. **is_class_template**
- **描述**: 是否为类模板。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
- **示例**:
  ```sql
  CREATE TABLE is_class_template (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES usertype(id)
  );
  ```

### 102. **class_instantiation**
- **描述**: 类实例化。
- **属性**:
  - `to` (INTEGER, 外键): 目标用户类型的ID，引用自`usertype`表。
  - `from` (INTEGER, 外键): 来源用户类型的ID，引用自`usertype`表。
- **示例**:
  ```sql
  CREATE TABLE class_instantiation (
      to INTEGER,
      from INTEGER,
      PRIMARY KEY (to, from),
      FOREIGN KEY (to) REFERENCES usertype(id),
      FOREIGN KEY (from) REFERENCES usertype(id)
  );
  ```

### 103. **class_template_argument**
- **描述**: 类模板参数。
- **属性**:
  - `type_id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
  - `index` (INTEGER): 参数索引。
  - `arg_type` (INTEGER, 外键): 参数类型的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE class_template_argument (
      type_id INTEGER,
      index INTEGER,
      arg_type INTEGER,
      PRIMARY KEY (type_id, index),
      FOREIGN KEY (type_id) REFERENCES usertype(id),
      FOREIGN KEY (arg_type) REFERENCES type(id)
  );
  ```

### 104. **class_template_argument_value**
- **描述**: 类模板参数值。
- **属性**:
  - `type_id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
  - `index` (INTEGER): 参数索引。
  - `arg_value` (INTEGER, 外键): 参数值的ID，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE class_template_argument_value (
      type_id INTEGER,
      index INTEGER,
      arg_value INTEGER,
      PRIMARY KEY (type_id, index),
      FOREIGN KEY (type_id) REFERENCES usertype(id),
      FOREIGN KEY (arg_value) REFERENCES expr(id)
  );
  ```

### 105. **user_or_decltype**
```
@user_or_decltype = @usertype | @decltype +-+
```
- **描述**: 用户的类型，可以是用户类型（`@usertype`）或声明类型（`@decltype`）。
- **属性**:
  - `id` (INTEGER): 主键
  - `associate_id` (INTEGER): 关联的变量ID
  - `type` (INTEGER): 类型
    - 1: @usertype
    - 2: @decltype

- **示例**:
  ```sql
  CREATE TBALE user_or_decltype (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 106. **is_proxy_class_for**
- **描述**: 是否为代理类。
- **属性**:
  - `id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
  - `templ_param_id` (INTEGER, 外键): 模板参数的ID，引用自`user_or_decltype`表。
- **示例**:
  ```sql
  CREATE TABLE is_proxy_class_for (
      id INTEGER PRIMARY KEY,
      templ_param_id INTEGER,
      FOREIGN KEY (id) REFERENCES usertype(id),
      FOREIGN KEY (templ_param_id) REFERENCES user_or_decltype(id)
  );
  ```

### 107. **type_mentions**
- **描述**: 类型提及。
- **属性**:
  - `id` (INTEGER, 主键): 类型提及的唯一标识符
  - `type_id` (INTEGER, 外键): 类型的ID，引用自`type`表。
  - `location` (INTEGER, 外键): 位置的ID，引用自`location`表。
  - `kind` (INTEGER): 种类。 // a_symbol_reference_kind from the frontend. ?Q
- **示例**:
  ```sql
  CREATE TABLE type_mentions (
      id INTEGER PRIMARY KEY,
      type_id INTEGER,
      location INTEGER,
      kind INTEGER,
      FOREIGN KEY (type_id) REFERENCES type(id),
      FOREIGN KEY (location) REFERENCES location(id)
  );
  ```

### 108. **is_function_template**
- **描述**: 是否为函数模板。
- **属性**:
  - `id` (INTEGER, 主键): 函数的唯一标识符，引用自`function`表。
- **示例**:
  ```sql
  CREATE TABLE is_function_template (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES function(id)
  );
  ```

### 109. **function_instantiation**
- **描述**: 函数实例化。
- **属性**:
  - `to` (INTEGER, 外键): 目标函数的ID，引用自`function`表。
  - `from` (INTEGER, 外键): 来源函数的ID，引用自`function`表。
- **示例**:
  ```sql
  CREATE TABLE function_instantiation (
      to INTEGER,
      from INTEGER,
      PRIMARY KEY (to, from),
      FOREIGN KEY (to) REFERENCES function(id),
      FOREIGN KEY (from) REFERENCES function(id)
  );
  ```

### 110. **function_template_argument**
- **描述**: 函数模板参数。
- **属性**:
  - `function_id` (INTEGER, 主键): 函数的唯一标识符，引用自`function`表。
  - `index` (INTEGER): 参数索引。
  - `arg_type` (INTEGER, 外键): 参数类型的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE function_template_argument (
      function_id INTEGER,
      index INTEGER,
      arg_type INTEGER,
      PRIMARY KEY (function_id, index),
      FOREIGN KEY (function_id) REFERENCES function(id),
      FOREIGN KEY (arg_type) REFERENCES type(id)
  );
  ```

### 111. **function_template_argument_value**
- **描述**: 函数模板参数值。
- **属性**:
  - `function_id` (INTEGER, 主键): 函数的唯一标识符，引用自`function`表。
  - `index` (INTEGER): 参数索引。
  - `arg_value` (INTEGER, 外键): 参数值的ID，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE function_template_argument_value (
      function_id INTEGER,
      index INTEGER,
      arg_value INTEGER,
      PRIMARY KEY (function_id, index),
      FOREIGN KEY (function_id) REFERENCES function(id),
      FOREIGN KEY (arg_value) REFERENCES expr(id)
  );
  ```

### 112. **is_variable_template**
- **描述**: 是否为变量模板。
- **属性**:
  - `id` (INTEGER, 主键): 变量的唯一标识符，引用自`variable`表。
- **示例**:
  ```sql
  CREATE TABLE is_variable_template (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES variable(id)
  );
  ```

### 113. **variable_instantiation**
- **描述**: 变量实例化。
- **属性**:
  - `to` (INTEGER, 外键): 目标变量的ID，引用自`variable`表。
  - `from` (INTEGER, 外键): 来源变量的ID，引用自`variable`表。
- **示例**:
  ```sql
  CREATE TABLE variable_instantiation (
      to INTEGER,
      from INTEGER,
      PRIMARY KEY (to, from),
      FOREIGN KEY (to) REFERENCES variable(id),
      FOREIGN KEY (from) REFERENCES variable(id)
  );
  ```

### 114. **variable_template_argument**
- **描述**: 变量模板参数。
- **属性**:
  - `variable_id` (INTEGER, 主键): 变量的唯一标识符，引用自`variable`表。
  - `index` (INTEGER): 参数索引。
  - `arg_type` (INTEGER, 外键): 参数类型的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE variable_template_argument (
      variable_id INTEGER,
      index INTEGER,
      arg_type INTEGER,
      PRIMARY KEY (variable_id, index),
      FOREIGN KEY (variable_id) REFERENCES variable(id),
      FOREIGN KEY (arg_type) REFERENCES type(id)
  );
  ```

### 115. **variable_template_argument_value**
- **描述**: 变量模板参数值。
- **属性**:
  - `variable_id` (INTEGER, 主键): 变量的唯一标识符，引用自`variable`表。
  - `index` (INTEGER): 参数索引。
  - `arg_value` (INTEGER, 外键): 参数值的ID，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE variable_template_argument_value (
      variable_id INTEGER,
      index INTEGER,
      arg_value INTEGER,
      PRIMARY KEY (variable_id, index),
      FOREIGN KEY (variable_id) REFERENCES variable(id),
      FOREIGN KEY (arg_value) REFERENCES expr(id)
  );
  ```

### 116. **template_template_instantiation**
- **描述**: 模板模板实例化。
- **属性**:
  - `to` (INTEGER, 外键): 目标用户类型的ID，引用自`usertype`表。
  - `from` (INTEGER, 外键): 来源用户类型的ID，引用自`usertype`表。
- **示例**:
  ```sql
  CREATE TABLE template_template_instantiation (
      to INTEGER,
      from INTEGER,
      PRIMARY KEY (to, from),
      FOREIGN KEY (to) REFERENCES usertype(id),
      FOREIGN KEY (from) REFERENCES usertype(id)
  );
  ```

### 117. **template_template_argument**
- **描述**: 模板模板参数。
- **属性**:
  - `type_id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
  - `index` (INTEGER): 参数索引。
  - `arg_type` (INTEGER, 外键): 参数类型的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE template_template_argument (
      type_id INTEGER,
      index INTEGER,
      arg_type INTEGER,
      PRIMARY KEY (type_id, index),
      FOREIGN KEY (type_id) REFERENCES usertype(id),
      FOREIGN KEY (arg_type) REFERENCES type(id)
  );
  ```

### 118. **template_template_argument_value**
- **描述**: 模板模板参数值。
- **属性**:
  - `type_id` (INTEGER, 主键): 用户类型的唯一标识符，引用自`usertype`表。
  - `index` (INTEGER): 参数索引。
  - `arg_value` (INTEGER, 外键): 参数值的ID，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE template_template_argument_value (
      type_id INTEGER,
      index INTEGER,
      arg_value INTEGER,
      PRIMARY KEY (type_id, index),
      FOREIGN KEY (type_id) REFERENCES usertype(id),
      FOREIGN KEY (arg_value) REFERENCES expr(id)
  );
  ```

### 119. **concept**
```
@concept = @concept_template | @concept_id +-+
```

- **描述**: 概念。
- **属性**:
  - `id`: 概念的 ID。
  - `associate_id`: 关联的 ID。
  - `type`: 子表类型
    - 1: @concept_template
    - 2: @concept_id
- **示例**:
  ```sql
  CREATE TBALE concept (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 120. **concept_templates**
- **描述**: 概念模板。
- **属性**:
  - `concept_id` (INTEGER, 主键): 概念模板的唯一标识符。
  - `name` (TEXT): 名称。
  - `location` (INTEGER, 外键): 位置的ID，引用自`location_default`表。
- **示例**:
  ```sql
  CREATE TABLE concept_templates (
      concept_id INTEGER PRIMARY KEY,
      name TEXT,
      location INTEGER,
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 121. **concept_instantiation**
- **描述**: 概念实例化。
- **属性**:
  - `to` (INTEGER, 外键): 目标概念的唯一标识符，引用自`concept_id`表。
  - `from` (INTEGER, 外键): 来源概念模板的唯一标识符，引用自`concept_template`表。
- **示例**:
  ```sql
  CREATE TABLE concept_instantiation (
      to INTEGER,
      from INTEGER,
      PRIMARY KEY (to, from),
      FOREIGN KEY (to) REFERENCES concept_id(id),
      FOREIGN KEY (from) REFERENCES concept_template(id)
  );
  ```

### 122. **is_type_constraint**
- **描述**: 是否为类型约束。
- **属性**:
  - `concept_id` (INTEGER, 主键): 概念的唯一标识符，引用自`concept_id`表。
- **示例**:
  ```sql
  CREATE TABLE is_type_constraint (
      concept_id INTEGER PRIMARY KEY,
      FOREIGN KEY (concept_id) REFERENCES concept_id(id)
  );
  ```

### 123. **concept_template_argument**
- **描述**: 概念模板参数。
- **属性**:
  - `concept_id` (INTEGER, 主键): 概念的唯一标识符，引用自`concept`表。
  - `index` (INTEGER): 参数索引。
  - `arg_type` (INTEGER, 外键): 参数类型的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE concept_template_argument (
      concept_id INTEGER,
      index INTEGER,
      arg_type INTEGER,
      PRIMARY KEY (concept_id, index),
      FOREIGN KEY (concept_id) REFERENCES concept(id),
      FOREIGN KEY (arg_type) REFERENCES type(id)
  );
  ```

### 124. **concept_template_argument_value**
- **描述**: 概念模板参数值。
- **属性**:
  - `concept_id` (INTEGER, 主键): 概念的唯一标识符，引用自`concept`表。
  - `index` (INTEGER): 参数索引。
  - `arg_value` (INTEGER, 外键): 参数值的ID，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE concept_template_argument_value (
      concept_id INTEGER,
      index INTEGER,
      arg_value INTEGER,
      PRIMARY KEY (concept_id, index),
      FOREIGN KEY (concept_id) REFERENCES concept(id),
      FOREIGN KEY (arg_value) REFERENCES expr(id)
  );
  ```

### 125. **routinetypes** :DONE
- **描述**: 例程类型。
- **属性**:
  - `id` (INTEGER, 主键): 例程类型的唯一标识符。
  - `return_type` (INTEGER, 外键): 返回类型的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE routinetypes (
      id INTEGER PRIMARY KEY,
      return_type INTEGER,
      FOREIGN KEY (return_type) REFERENCES type(id)
  );
  ```

### 126. **routinetypeargs** :DONE
- **描述**: 例程类型参数。
- **属性**:
  - `routine` (INTEGER, 外键): 例程类型的ID，引用自`routinetype`表。
  - `index` (INTEGER): 参数索引。
  - `type_id` (INTEGER, 外键): 参数类型的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE routinetypeargs (
      routine INTEGER,
      index INTEGER,
      type_id INTEGER,
      PRIMARY KEY (routine, index),
      FOREIGN KEY (routine) REFERENCES routinetype(id),
      FOREIGN KEY (type_id) REFERENCES type(id)
  );
  ```

### 127. **ptrtomembers** :DONE
- **描述**: 指向成员的指针。
- **属性**:
  - `id` (INTEGER, 主键): 指向成员的指针的唯一标识符。
  - `type_id` (INTEGER, 外键): 类型的ID，引用自`type`表。
  - `class_id` (INTEGER, 外键): 类的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE ptrtomembers (
      id INTEGER PRIMARY KEY,
      type_id INTEGER,
      class_id INTEGER,
      FOREIGN KEY (type_id) REFERENCES type(id),
      FOREIGN KEY (class_id) REFERENCES type(id)
  );
  ```

### 128. **specifiers**
- **描述**: 类型的、函数的和变量的说明符。
- **属性**:

  - `id` (INTEGER, 主键): 说明符的唯一标识符。
  - `str` (TEXT, 主键): 说明符的字符串表示。

    ```
    "public",
    "protected",
    "private",

    "const",
    "volatile",
    "static",

    "pure",
    "virtual",
    "sealed", // Microsoft
    "__interface", // Microsoft
    "inline",
    "explicit",

    "near", // near far extension
    "far", // near far extension
    "__ptr32", // Microsoft
    "__ptr64", // Microsoft
    "__sptr", // Microsoft
    "__uptr", // Microsoft
    "dllimport", // Microsoft
    "dllexport", // Microsoft
    "thread", // Microsoft
    "naked", // Microsoft
    "microsoft_inline", // Microsoft
    "forceinline", // Microsoft
    "selectany", // Microsoft
    "nothrow", // Microsoft
    "novtable", // Microsoft
    "noreturn", // Microsoft
    "noinline", // Microsoft
    "noalias", // Microsoft
    "restrict", // Microsoft
    ```

- **示例**:
  ```sql
  CREATE TABLE specifiers (
      id INTEGER PRIMARY KEY,
      str TEXT
  );
  ```

### 129. **typespecifiers**
- **描述**: 类型的说明符。
- **属性**:
  - `type_id` (INTEGER, 外键): 类型的ID，引用自`type`表。
  - `spec_id` (INTEGER, 外键): 说明符的ID，引用自`specifier`表。
- **示例**:
  ```sql
  CREATE TABLE typespecifiers (
      type_id INTEGER,
      spec_id INTEGER,
      PRIMARY KEY (type_id, spec_id),
      FOREIGN KEY (type_id) REFERENCES type(id),
      FOREIGN KEY (spec_id) REFERENCES specifier(id)
  );
  ```

### 130. **funspecifiers**
- **描述**: 函数的说明符。
- **属性**:
  - `func_id` (INTEGER, 外键): 函数的ID，引用自`function`表。
  - `spec_id` (INTEGER, 外键): 说明符的ID，引用自`specifier`表。
- **示例**:
  ```sql
  CREATE TABLE funspecifiers (
      func_id INTEGER,
      spec_id INTEGER,
      PRIMARY KEY (func_id, spec_id),
      FOREIGN KEY (func_id) REFERENCES function(id),
      FOREIGN KEY (spec_id) REFERENCES specifier(id)
  );
  ```

### 131. **varspecifiers**
- **描述**: 变量的说明符。
- **属性**:
  - `var_id` (INTEGER, 外键): 变量的ID，引用自`accessible`表。
  - `spec_id` (INTEGER, 外键): 说明符的ID，引用自`specifier`表。
- **示例**:
  ```sql
  CREATE TABLE varspecifiers (
      var_id INTEGER,
      spec_id INTEGER,
      PRIMARY KEY (var_id, spec_id),
      FOREIGN KEY (var_id) REFERENCES accessible(id),
      FOREIGN KEY (spec_id) REFERENCES specifier(id)
  );
  ```

### 132. **explicit_specifier_exprs**
- **描述**: 明确的说明符表达式。
- **属性**:
  - `func_id` (INTEGER, 主键): 函数的ID，引用自`function`表。
  - `constant` (INTEGER, 外键): 常量的ID，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE explicit_specifier_exprs (
      func_id INTEGER PRIMARY KEY,
      constant INTEGER,
      FOREIGN KEY (func_id) REFERENCES function(id),
      FOREIGN KEY (constant) REFERENCES expr(id)
  );
  ```

### 133. **attributes**
- **描述**: 属性。
- **属性**:

  - `id` (INTEGER, 主键): 属性的唯一标识符。
  - `kind` (INTEGER): 属性的种类。

    ```
    case @attribute.kind of
      0 = @gnuattribute
    | 1 = @stdattribute
    | 2 = @declspec
    | 3 = @msattribute
    | 4 = @alignas
    // ... 5 @objc_propertyattribute deprecated
    ;
    ```

  - `name` (TEXT): 属性名。
  - `name_space` (TEXT): 命名空间。
  - `location` (INTEGER, 外键): 位置的ID，引用自`location_default`表。

- **示例**:
  ```sql
  CREATE TABLE attributes (
      id INTEGER PRIMARY KEY,
      kind INTEGER,
      name TEXT,
      name_space TEXT,
      location INTEGER,
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 134. **attribute_args**
- **描述**: 属性参数。
- **属性**:

  - `id` (INTEGER, 主键): 属性参数的唯一标识符。
  - `kind` (INTEGER): 参数的种类。

    ```
    case @attribute_arg.kind of
      0 = @attribute_arg_empty
    | 1 = @attribute_arg_token
    | 2 = @attribute_arg_constant
    | 3 = @attribute_arg_type
    | 4 = @attribute_arg_constant_expr
    | 5 = @attribute_arg_expr
    ;
    ```

  - `attribute` (INTEGER, 外键): 属性的ID，引用自`attribute`表。
  - `index` (INTEGER): 参数索引。
  - `location` (INTEGER, 外键): 位置的ID，引用自`location_default`表。

- **示例**:
  ```sql
  CREATE TABLE attribute_args (
      id INTEGER PRIMARY KEY,
      kind INTEGER,
      attribute INTEGER,
      index INTEGER,
      location INTEGER,
      FOREIGN KEY (attribute) REFERENCES attribute(id),
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 135. **attribute_arg_value**
- **描述**: 属性参数值。
- **属性**:
  - `arg` (INTEGER, 主键): 属性参数的唯一标识符，引用自`attribute_arg`表。
  - `value` (TEXT): 参数值。
- **示例**:
  ```sql
  CREATE TABLE attribute_arg_value (
      arg INTEGER PRIMARY KEY,
      value TEXT,
      FOREIGN KEY (arg) REFERENCES attribute_arg(id)
  );
  ```

### 136. **attribute_arg_type**
- **描述**: 属性参数类型。
- **属性**:
  - `arg` (INTEGER, 主键): 属性参数的唯一标识符，引用自`attribute_arg`表。
  - `type_id` (INTEGER, 外键): 类型的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE attribute_arg_type (
      arg INTEGER PRIMARY KEY,
      type_id INTEGER,
      FOREIGN KEY (arg) REFERENCES attribute_arg(id),
      FOREIGN KEY (type_id) REFERENCES type(id)
  );
  ```

### 137. **attribute_arg_constant**
- **描述**: 属性参数常量。
- **属性**:
  - `arg` (INTEGER, 主键): 属性参数的唯一标识符，引用自`attribute_arg`表。
  - `constant` (INTEGER, 外键): 常量的ID，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE attribute_arg_constant (
      arg INTEGER PRIMARY KEY,
      constant INTEGER,
      FOREIGN KEY (arg) REFERENCES attribute_arg(id),
      FOREIGN KEY (constant) REFERENCES expr(id)
  );
  ```

### 138. **attribute_arg_expr**
- **描述**: 属性参数表达式。
- **属性**:
  - `arg` (INTEGER, 主键): 属性参数的唯一标识符，引用自`attribute_arg`表。
  - `expr` (INTEGER, 外键): 表达式的ID，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE attribute_arg_expr (
      arg INTEGER PRIMARY KEY,
      expr INTEGER,
      FOREIGN KEY (arg) REFERENCES attribute_arg(id),
      FOREIGN KEY (expr) REFERENCES expr(id)
  );
  ```

### 139. **attribute_arg_name**
- **描述**: 属性参数名称。
- **属性**:
  - `arg` (INTEGER, 主键): 属性参数的唯一标识符，引用自`attribute_arg`表。
  - `name` (TEXT): 参数名。
- **示例**:
  ```sql
  CREATE TABLE attribute_arg_name (
      arg INTEGER PRIMARY KEY,
      name TEXT,
      FOREIGN KEY (arg) REFERENCES attribute_arg(id)
  );
  ```

### 140. **typeattributes**
- **描述**: 类型的属性。
- **属性**:
  - `type_id` (INTEGER, 外键): 类型的ID，引用自`type`表。
  - `spec_id` (INTEGER, 外键): 属性ID，引用自`attribute`表。
- **示例**:
  ```sql
  CREATE TABLE typeattributes (
      type_id INTEGER,
      spec_id INTEGER,
      PRIMARY KEY (type_id, spec_id),
      FOREIGN KEY (type_id) REFERENCES type(id),
      FOREIGN KEY (spec_id) REFERENCES attribute(id)
  );
  ```

### 141. **funcattributes**
- **描述**: 函数的属性。
- **属性**:
  - `func_id` (INTEGER, 外键): 函数的ID，引用自`function`表。
  - `spec_id` (INTEGER, 外键): 属性ID，引用自`attribute`表。
- **示例**:
  ```sql
  CREATE TABLE funcattributes (
      func_id INTEGER,
      spec_id INTEGER,
      PRIMARY KEY (func_id, spec_id),
      FOREIGN KEY (func_id) REFERENCES function(id),
      FOREIGN KEY (spec_id) REFERENCES attribute(id)
  );
  ```

### 142. **varattributes**
- **描述**: 变量的属性。
- **属性**:
  - `var_id` (INTEGER, 外键): 变量的ID，引用自`accessible`表。
  - `spec_id` (INTEGER, 外键): 属性ID，引用自`attribute`表。
- **示例**:
  ```sql
  CREATE TABLE varattributes (
      var_id INTEGER,
      spec_id INTEGER,
      PRIMARY KEY (var_id, spec_id),
      FOREIGN KEY (var_id) REFERENCES accessible(id),
      FOREIGN KEY (spec_id) REFERENCES attribute(id)
  );
  ```

### 143. **stmtattributes**
- **描述**: 语句的属性。
- **属性**:
  - `stmt_id` (INTEGER, 外键): 语句ID，引用自`stmt`表。
  - `spec_id` (INTEGER, 外键): 属性ID，引用自`attribute`表。
- **示例**:
  ```sql
  CREATE TABLE stmtattributes (
      stmt_id INTEGER,
      spec_id INTEGER,
      PRIMARY KEY (stmt_id, spec_id),
      FOREIGN KEY (stmt_id) REFERENCES stmt(id),
      FOREIGN KEY (spec_id) REFERENCES attribute(id)
  );
  ```

### 144. **type** :DONE
```
@type = @builtintype +-+
      | @derivedtype
      | @usertype
      /* TODO | @fixedpointtype */
      | @routinetype
      | @ptrtomember
      | @decltype;
```
- **描述**: 类型。
- **属性**:
  - `id`: 类型的 ID。
  - `associate_id`: 关联的子表 ID。
  - `type`: 子表类型
    - 1: @buildintype
    - 2: @derivedtype
    - 3: @usertype
    - 4: @routinetype
    - 5: @ptrtomember
    - 6: @decltype
- **示例**:
  ```sql
  CREATE TBALE type (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 145. **unspecifiedtype**
- **描述**: 未指定的类型。
- **属性**:
  - `type_id` (INTEGER, 主键): 类型的ID，引用自`type`表。
  - `unspecified_type_id` (INTEGER, 外键): 未指定的类型的ID，引用自`type`表。
- **示例**:
  ```sql
  CREATE TABLE unspecifiedtype (
      type_id INTEGER PRIMARY KEY,
      unspecified_type_id INTEGER,
      FOREIGN KEY (type_id) REFERENCES type(id),
      FOREIGN KEY (unspecified_type_id) REFERENCES type(id)
  );
  ```

### 146. **member** :TODO :?
- **描述**: 成员。
- **属性**:
  - `parent` (INTEGER, 外键): 父类型的ID，引用自`type`表。
  - `index` (INTEGER): 索引。
  - `child` (INTEGER, 主键): 成员的ID，引用自`member`表。
- **示例**:
  ```sql
  CREATE TABLE member (
      parent INTEGER,
      index INTEGER,
      child INTEGER PRIMARY KEY,
      FOREIGN KEY (parent) REFERENCES type(id),
      FOREIGN KEY (child) REFERENCES member(id)
  );
  ```

### 147. **enclosingfunction_child**
```
@enclosingfunction_child = @usertype | @variable | @namespace +-+
```
- **描述**: 引用了当前函数的函数。
- **属性**:
  - `id`:  ID。
  - `associate_id`: 关联的子表 ID。
  - `type`: 子表类型
    - 1: @usertype
    - 2: @variable
    - 3: @namespace
- **示例**:
  ```sql
  CREATE TBALE enclosingfunction_child (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 148. **enclosingfunction**
- **描述**: 包含函数。
- **属性**:
  - `child` (INTEGER, 主键): 子元素的ID，引用自`enclosingfunction_child`表。
  - `parent` (INTEGER, 外键): 函数的ID，引用自`function`表。
- **示例**:
  ```sql
  CREATE TABLE enclosingfunction (
      child INTEGER PRIMARY KEY,
      parent INTEGER,
      FOREIGN KEY (child) REFERENCES user_or_decltype(id),
      FOREIGN KEY (parent) REFERENCES function(id)
  );
  ```

### 149. **derivations**
- **描述**: 派生。
- **属性**:
  - `derivation` (INTEGER, 主键): 派生的唯一标识符。
  - `sub` (INTEGER, 外键): 子类型的ID，引用自`type`表。
  - `index` (INTEGER): 索引。
  - `super` (INTEGER, 外键): 父类型的ID，引用自`type`表。
  - `location` (INTEGER, 外键): 位置的ID，引用自`location_default`表。
- **示例**:
  ```sql
  CREATE TABLE derivations (
      derivation INTEGER PRIMARY KEY,
      sub INTEGER,
      index INTEGER,
      super INTEGER,
      location INTEGER,
      FOREIGN KEY (sub) REFERENCES type(id),
      FOREIGN KEY (super) REFERENCES type(id),
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 150. **derspecifiers**
- **描述**: 派生说明符。
- **属性**:
  - `der_id` (INTEGER, 主键): 派生ID，引用自`derivation`表。
  - `spec_id` (INTEGER, 外键): 说明符ID，引用自`specifier`表。
- **示例**:
  ```sql
  CREATE TABLE derspecifiers (
      der_id INTEGER PRIMARY KEY,
      spec_id INTEGER,
      FOREIGN KEY (der_id) REFERENCES derivation(id),
      FOREIGN KEY (spec_id) REFERENCES specifier(id)
  );
  ```

### 151. **direct_base_offsets**
- **描述**: 直接基类偏移量。
- **属性**:
  - `der_id` (INTEGER, 主键): 派生ID，引用自`derivation`表。
  - `offset` (INTEGER): 偏移量。
- **示例**:
  ```sql
  CREATE TABLE direct_base_offsets (
      der_id INTEGER PRIMARY KEY,
      offset INTEGER,
      FOREIGN KEY (der_id) REFERENCES derivation(id)
  );
  ```

### 152. **virtual_base_offsets**
- **描述**: 虚拟基类偏移量。
- **属性**:
  - `sub` (INTEGER, 外键): 子用户类型的ID，引用自`usertype`表。
  - `super` (INTEGER, 外键): 父用户类型的ID，引用自`usertype`表。
  - `offset` (INTEGER): 偏移量。
- **示例**:
  ```sql
  CREATE TABLE virtual_base_offsets (
      sub INTEGER,
      super INTEGER,
      offset INTEGER,
      PRIMARY KEY (sub, super),
      FOREIGN KEY (sub) REFERENCES usertype(id),
      FOREIGN KEY (super) REFERENCES usertype(id)
  );
  ```

### 153. **frienddecls**
- **描述**: 友元声明。
- **属性**:
  - `id` (INTEGER, 主键): 友元声明的唯一标识符。
  - `type_id` (INTEGER, 外键): 类型的ID，引用自`type`表。
  - `decl_id` (INTEGER, 外键): 声明的ID，引用自`declaration`表。
  - `location` (INTEGER, 外键): 位置的ID，引用自`location_default`表。
- **示例**:
  ```sql
  CREATE TABLE frienddecls (
      id INTEGER PRIMARY KEY,
      type_id INTEGER,
      decl_id INTEGER,
      location INTEGER,
      FOREIGN KEY (type_id) REFERENCES type(id),
      FOREIGN KEY (decl_id) REFERENCES declaration(id),
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 154. **declaredtype**
```
@declaredtype = @usertype; +-+
```
- **description**: The declared type of a declaration.
- **属性**:
  - `id`:  ID。
  - `associate_id`: 关联的 ID。
  - `type`: 子表类型
    - 1: @usertype
- **示例**:
  ```sql
  CREATE TBALE declaredtype (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```


### 155. **declaration** :PARTIALLYDONE
```
@declaration = @function +-+
| @declaredtype
| @variable
| @enumconstant
| @frienddecl
| @concept_template;
```
- **description**: The declared type of a declaration.
- **属性**:
  - `id`:  ID。
  - `associate_id`: 关联的 ID。
  - `type`: 子表类型
    - 1: @function
    - 2: @declaredtype
    - 3: @variable
    - 4: @enumconstant
    - 5: @frienddecl
    - 6: @concept_template
- **示例**:
  ```sql
  CREATE TBALE declaration (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 156. **member** :PARTIALLYDONE
```
@member = @membervariable +-+
| @function
| @declaredtype
| @enumconstant;
```
- **description**: 成员变量
- **属性**:
  - `id`:  ID。
  - `associate_id`: 关联的 ID。
  - `type`: 子表类型
    - 1: @memebervariable
    - 2: @function
    - 3: @declaredtype
    - 4: @enumconstant
- **示例**:
  ```sql
  CREATE TBALE memeber (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 157. **locatable**
```
@locatable = @diagnostic +-+
| @declaration
| @ppd_include
| @ppd_define
| @macroinvocation
/*| @funcall\_/
| @xmllocatable
| @attribute
| @attribute_arg;
```
- **description**: 表示可定位的元素。
- **属性**:
  - `id`:  ID。
  - `associate_id`: 关联的 ID。
  - `type`: 子表类型
    - 1: @diagnostic
    - 2: @declaration
    - 3: @ppd_include
    - 4: @ppd_define
    - 5: @macroinvocation
    - 6: @xmllocatable
    - 7: @attribute
    - 8: @attribute_arg
- **示例**:
  ```sql
  CREATE TBALE locatable (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 158. **namedscope**
```
@namedscope = @namespace | @usertype; +-+
```
- **description**: 命名空间或用户类型。
- **属性**:
  - `id`:  ID。
  - `associate_id`: 关联的 ID。
  - `type`: 子表类型
    - 1: @namespace
    - 2: @usertype
- **示例**:
  ```sql
  CREATE TBALE namedscope (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 159. **element**
```
@element = @locatable +-+
| @file
| @folder
| @specifier
| @type
| @expr
| @namespace
| @initialiser
| @stmt
| @derivation
| @comment
| @preprocdirect
| @fun_decl
| @var_decl
| @type_decl
| @namespace_decl
| @using
| @namequalifier
| @specialnamequalifyingelement
| @static_assert
| @type_mention
| @lambdacapture;
```
- **description**: 元素。
- **属性**:
  - `id`:  ID。
  - `associate_id`: 关联的 ID。
  - `type`: 子表类型
    - 1: @file
    - 2: @folder
    - 3: @specifier
    - 4: @type
    - 5: @expr
    - 6: @namespace
    - 7: @initialiser
    - 8: @stmt
    - 9: @derivation
    - 10: @commment
    - 11: @preprocdirect
    - 12: @fun_decl
    - 13: @var_decl
    - 14: @type_decl
    - 15: @namespace_decl
    - 16: @using
    - 17: @namequalifier
    - 18: @specialnamequalifyingelement
    - 19: @static_assert
    - 20: @type_mention
    - 21: @lambdacapture;
- **示例**:
  ```sql
  CREATE TBALE element (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 160. **exprparent**
```
@exprparent = @element; +-+
```
- **description**: 表示一个表达式的父节点。
- **属性**:
  - `id`:  ID。
  - `associate_id`: 关联的 ID。
  - `type`: 子表类型
    - 1: @element
- **示例**:
  ```sql
  CREATE TBALE exprparent (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 161. **comments**
- **描述**: 注释。
- **属性**:
  - `id` (INTEGER, 主键): 唯一标识符，引用自`comment`。
  - `contents` (TEXT): 注释内容
  - `location` (INTEGER, 外键): 位置，引用自`location_default`表。
- **示例**:
  ```sql
  CREATE TABLE comments (
      id INTEGER PRIMARY KEY,
      contents TEXT,
      location INTEGER,
      FOREIGN KEY (location) REFERENCES location_default(id)
  );
  ```

### 162. **commentbinding**
- **描述**: 注释绑定。
- **属性**:
  - `id` (INTEGER, 主键): 唯一标识符，引用自`comment`。
  - `element` (INTEGER, 外键): 元素，引用自`element`表。
- **示例**:
  ```sql
  CREATE TABLE commentbinding (
      id INTEGER PRIMARY KEY,
      element INTEGER,
      FOREIGN KEY (id) REFERENCES comments(id),
      FOREIGN KEY (element) REFERENCES element(id)
  );
  ```

### 163. **exprconv**
- **描述**: 表达式转换。
- **属性**:
  - `converted` (INTEGER, 外键): 转换后的表达式，引用自`expr`表。
  - `conversion` (INTEGER, 外键): 转换表达式，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE exprconv (
      converted INTEGER,
      conversion INTEGER,
      PRIMARY KEY (converted, conversion),
      FOREIGN KEY (converted) REFERENCES expr(id),
      FOREIGN KEY (conversion) REFERENCES expr(id)
  );
  ```

### 164. **compgenerated**
- **描述**: 编译器生成。
- **属性**:
  - `id` (INTEGER, 主键): 唯一标识符，引用自`element`。
- **示例**:
  ```sql
  CREATE TABLE compgenerated (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES element(id)
  );
  ```

### 165. **synthetic_destructor_call**
- **描述**: 合成析构函数调用。
  > `destructor_call` destructs the `i`'th entity that should be destructed following `element`. Note that entities should be destructed in reverse construction order, so for a given `element` these should be called from highest to lowest `i`.
- **属性**:
  - `element` (INTEGER, 外键): 元素，引用自`element`表。
  - `i` (INTEGER): 索引。
  - `destructor_call` (INTEGER, 外键): 析构函数调用，引用自`routineexpr`表。
- **示例**:
  ```sql
  CREATE TABLE synthetic_destructor_call (
      element INTEGER,
      i INTEGER,
      destructor_call INTEGER,
      PRIMARY KEY (element, i),
      PRIMARY KEY (element, destructor_call)
      FOREIGN KEY (element) REFERENCES element(id),
      FOREIGN KEY (destructor_call) REFERENCES routineexpr(id)
  );
  ```

### 166. **namespaces**
- **描述**: 命名空间。
- **属性**:
  - `id` (INTEGER, 主键): 唯一标识符，引用自`namespace`。
  - `name` (TEXT): 名称
- **示例**:
  ```sql
  CREATE TABLE namespaces (
      id INTEGER PRIMARY KEY,
      name TEXT,
      FOREIGN KEY (id) REFERENCES namespace(id)
  );
  ```

### 167. **namespace_inline**
- **描述**: 内联命名空间。
- **属性**:
  - `id` (INTEGER, 主键): 唯一标识符，引用自`namespace`。
- **示例**:
  ```sql
  CREATE TABLE namespace_inline (
      id INTEGER PRIMARY KEY,
      FOREIGN KEY (id) REFERENCES namespace(id)
  );
  ```

### 168. **namespacembrs**
- **描述**: 命名空间成员。
- **属性**:
  - `parentid` (INTEGER, 外键): 父命名空间，引用自`namespace`表。
  - `memberid` (INTEGER, 外键): 成员，引用自`namespacembr`表。
- **示例**:
  ```sql
  CREATE TABLE namespacembrs (
      parentid INTEGER,
      memberid INTEGER,
      PRIMARY KEY (parentid, memberid),
      FOREIGN KEY (parentid) REFERENCES namespace(id),
      FOREIGN KEY (memberid) REFERENCES namespacembr(id)
  );
  ```

### 169. **namespacembr**
```
@namespacembr = @declaration | @namespace +-+
```
- **description**: 命名空间成员。
- **属性**:
  - `id`:  ID。
  - `associate_id`: 关联的 ID。
  - `type`: 子表类型
    - 1: @declaration
    - 2: @namespace
- **示例**:
  ```sql
  CREATE TBALE namespacembr (
    id INTEGER PRIMARY KEY,
    associate_id INTEGER,
    type INTEGER,
  )
  ```

### 170. **exprparents**
- **描述**: 表达式父节点。
- **属性**:
  - `expr_id` (INTEGER, 外键): 表达式，引用自`expr`表。
  - `child_index` (INTEGER): 子索引。
  - `parent_id` (INTEGER, 外键): 父表达式，引用自`exprparent`表。
- **示例**:
  ```sql
  CREATE TABLE exprparents (
      expr_id INTEGER,
      child_index INTEGER,
      parent_id INTEGER,
      PRIMARY KEY (expr_id, child_index),
      FOREIGN KEY (expr_id) REFERENCES expr(id),
      FOREIGN KEY (parent_id) REFERENCES exprparent(id)
  );
  ```

### 171. **expr_isload**
- **描述**: 表达式加载。
- **属性**:
  - `expr_id` (INTEGER, 主键): 表达式，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE expr_isload (
      expr_id INTEGER PRIMARY KEY,
      FOREIGN KEY (expr_id) REFERENCES expr(id)
  );
  ```

### 172. **cast**
```
@cast = @c_style_cast +-+
      | @const_cast
      | @dynamic_cast
      | @reinterpret_cast
      | @static_cast
      ;
```

### 173. **conversionkinds**
- **描述**: 转换类型。
- **属性**:
  - `expr_id` (INTEGER, 主键): 表达式，引用自`cast`表。
  - `kind` (INTEGER): 类型。
    ```
    case @conversion.kind of
      0 = @simple_conversion           // a numeric conversion, qualification conversion, or a reinterpret_cast
    | 1 = @bool_conversion             // conversion to 'bool'
    | 2 = @base_class_conversion       // a derived-to-base conversion
    | 3 = @derived_class_conversion    // a base-to-derived conversion
    | 4 = @pm_base_class_conversion    // a derived-to-base conversion of a pointer to member
    | 5 = @pm_derived_class_conversion // a base-to-derived conversion of a pointer to member
    | 6 = @glvalue_adjust              // an adjustment of the type of a glvalue
    | 7 = @prvalue_adjust              // an adjustment of the type of a prvalue
    ;
    ```
- **示例**:
  ```sql
  CREATE TABLE conversionkinds (
      expr_id INTEGER PRIMARY KEY,
      kind INTEGER,
      FOREIGN KEY (expr_id) REFERENCES cast(id)
  );
  ```

### 174. **conversion**
```
@conversion = @cast +-+
| @array_to_pointer
| @parexpr
| @reference_to
| @ref_indirect
| @temp_init
| @c11_generic
;
```

### 175. **iscall** :TODO

- **描述**: 表示一个函数调用。
- **属性**:
  - `caller` (INTEGER, 主键): 调用者的唯一标识符，引用自`funbindexpr`表。
  - `kind` (INTEGER): 调用的类型。

```sql
CREATE TABLE iscall (
    caller INTEGER PRIMARY KEY,
    kind INTEGER,
    FOREIGN KEY (caller) REFERENCES funbindexpr(ref)
);
```

```
/*
   kind(1) = mbrcallexpr
   kind(2) = mbrptrcallexpr
   kind(3) = mbrptrmbrcallexpr
   kind(4) = ptrmbrptrmbrcallexpr
   kind(5) = mbrreadexpr // x.y
   kind(6) = mbrptrreadexpr // p->y
   kind(7) = mbrptrmbrreadexpr // x.*pm
   kind(8) = mbrptrmbrptrreadexpr // x->*pm
   kind(9) = staticmbrreadexpr // static x.y
   kind(10) = staticmbrptrreadexpr // static p->y
*/
```

### 176. **numtemplatearguments**

- **描述**: 表示模板参数的数目。
- **属性**:
  - `expr_id` (INTEGER, 主键): 表达式的唯一标识符，引用自`expr`表。
  - `num` (INTEGER): 模板参数的数量。

```sql
CREATE TABLE numtemplatearguments (
    expr_id INTEGER PRIMARY KEY,
    num INTEGER,
    FOREIGN KEY (expr_id) REFERENCES expr(ref)
);
```

### 177. **specialnamequalifyingelements**

- **描述**: 表示特殊名称限定元素。
- **属性**:
  - `id` (INTEGER, 主键): 特殊名称限定元素的唯一标识符。
  - `name` (VARCHAR, 唯一): 元素的名称。

```sql
CREATE TABLE specialnamequalifyingelements (
    id INTEGER PRIMARY KEY,
    name VARCHAR(255) UNIQUE,
    FOREIGN KEY (id) REFERENCES specialnamequalifyingelement(id)
);
```

### 178. **namequalifiableelement**
```
@namequalifiableelement = @expr | @namequalifier;
```

### 179. **namequalifiableelement**
```
@namequalifyingelement = @namespace
                       | @specialnamequalifyingelement
                       | @usertype;
```

### 180. **namequalifiers**

- **描述**: 表示名称限定符。
- **属性**:
  - `id` (INTEGER, 主键): 名称限定符的唯一标识符，引用自`namequalifier`表。
  - `qualifiableelement` (INTEGER): 可限定元素的唯一标识符，引用自`namequalifiableelement`表。
  - `qualifyingelement` (INTEGER): 限定元素的唯一标识符，引用自`namequalifyingelement`表。
  - `location` (INTEGER): 位置的唯一标识符，引用自`location_default`表。

```sql
CREATE TABLE namequalifiers (
    id INTEGER PRIMARY KEY,
    qualifiableelement INTEGER,
    qualifyingelement INTEGER,
    location INTEGER,
    FOREIGN KEY (qualifiableelement) REFERENCES namequalifiableelement(ref),
    FOREIGN KEY (qualifyingelement) REFERENCES namequalifyingelement(ref),
    FOREIGN KEY (location) REFERENCES location_default(ref)
);
```

### 181. **varbind** :DONE

- **描述**: 表示变量绑定。
- **属性**:
  - `expr` (INTEGER, 主键): 表达式的唯一标识符，引用自`varbindexpr`表。
  - `var` (INTEGER): 可访问变量的唯一标识符，引用自`accessible`表。// Change to variable table

```sql
CREATE TABLE varbind (
    expr INTEGER PRIMARY KEY,
    var INTEGER,
    FOREIGN KEY (expr) REFERENCES varbindexpr(ref),
    FOREIGN KEY (var) REFERENCES accessible(ref)
);
```

### 182. **funbind** :TODO

- **描述**: 表示函数绑定。
- **属性**:
  - `expr` (INTEGER, 主键): 表达式的唯一标识符，引用自`funbindexpr`表。
  - `fun` (INTEGER): 函数唯一标识符，引用自`function`表。

```sql
CREATE TABLE funbind (
    expr INTEGER PRIMARY KEY,
    fun INTEGER,
    FOREIGN KEY (expr) REFERENCES funbindexpr(ref),
    FOREIGN KEY (fun) REFERENCES function(ref)
);
```

### 183. **any_new_expr**
```
@any_new_expr = @new_expr +-+
              | @new_array_expr;
```

### 184. **new_or_delete_expr**
```
@new_or_delete_expr = @any_new_expr
                    | @delete_expr
                    | @delete_array_expr;
```

### 185. **prefix_crement_expr**
```
@prefix_crement_expr = @preincrexpr | @predecrexpr;
```

### 186. **postfix_crement_expr**
```
@postfix_crement_expr = @postincrexpr | @postdecrexpr;
```

### 187. **increment_expr**
```
@increment_expr = @preincrexpr | @postincrexpr;
```

### 188. **decrement_expr**
```
@decrement_expr = @predecrexpr | @postdecrexpr;
```

### 189. **crement_expr** :TODO
```
@crement_expr = @increment_expr | @decrement_expr;
```

### 190. **un_arith_op_expr** :DONE
```
@un_arith_op_expr = @arithnegexpr
                  | @unaryplusexpr
                  | @conjugation
                  | @realpartexpr
                  | @imagpartexpr
                  | @crement_expr
                  ;
```

### 191. **un_bitwise_op_expr** :DONE
```
@un_bitwise_op_expr = @complementexpr;
```

### 192. **un_log_op_expr** :DONE
```
@un_log_op_expr = @notexpr;
```

### 193. **un_op_expr** :DONE
```
@un_op_expr = @address_of
            | @indirect
            | @un_arith_op_expr
            | @un_bitwise_op_expr
            | @builtinaddressof
            | @vec_fill
            | @un_log_op_expr
            | @co_await
            | @co_yield
            ;
```

### 194. **bin_log_op_expr** :DONE
```
@bin_log_op_expr = @andlogicalexpr | @orlogicalexpr;
```

### 195. **cmp_op_expr** :DONE
```
@cmp_op_expr = @eq_op_expr | @rel_op_expr;
```

### 196. **eq_op_expr** :DONE
```
@eq_op_expr = @eqexpr | @neexpr;
```

### 197. **rel_op_expr** :DONE
```
@rel_op_expr = @gtexpr
             | @ltexpr
             | @geexpr
             | @leexpr
             | @spaceshipexpr
             ;
```

### 198. **bin_bitwise_op_expr** :DONE
```
@bin_bitwise_op_expr = @lshiftexpr
                     | @rshiftexpr
                     | @andexpr
                     | @orexpr
                     | @xorexpr
                     ;
```

### 199. **p_arith_op_expr** :DONE
```
@p_arith_op_expr = @paddexpr
                 | @psubexpr
                 | @pdiffexpr
                 ;
```

### 200. **bin_arith_op_expr** :DONE
```
@bin_arith_op_expr = @addexpr
                   | @subexpr
                   | @mulexpr
                   | @divexpr
                   | @remexpr
                   | @jmulexpr
                   | @jdivexpr
                   | @fjaddexpr
                   | @jfaddexpr
                   | @fjsubexpr
                   | @jfsubexpr
                   | @minexpr
                   | @maxexpr
                   | @p_arith_op_expr
                   ;
```

### 201. **bin_op_expr** :DONE
```
@bin_op_expr = @bin_arith_op_expr
             | @bin_bitwise_op_expr
             | @cmp_op_expr
             | @bin_log_op_expr
             ;
```

### 202. **op_expr** :DOING
```
@op_expr = @un_op_expr
         | @bin_op_expr
         | @assign_expr
         | @conditionalexpr
         ;
```

### 203. **assign_arith_expr**
```
@assign_arith_expr = @assignaddexpr
                   | @assignsubexpr
                   | @assignmulexpr
                   | @assigndivexpr
                   | @assignremexpr
                   ;
```

### 204. **assign_bitwise_expr**
```
@assign_bitwise_expr = @assignandexpr
                     | @assignorexpr
                     | @assignxorexpr
                     | @assignlshiftexpr
                     | @assignrshiftexpr
                     ;
```

### 205. **assign_pointer_expr**
```
@assign_pointer_expr = @assignpaddexpr
                     | @assignpsubexpr
                     ;
```

### 206. **assign_op_expr**
```
@assign_op_expr = @assign_arith_expr
                | @assign_bitwise_expr
                | @assign_pointer_expr
                ;
```

### 207. **assign_expr**
```
@assign_expr = @assignexpr | @assign_op_expr | @blockassignexpr
```


### 208. **expr_allocator**
- **描述**: 表达式分配器。
- **属性**:
  - `expr` (INTEGER, 主键): 表达式，引用自`any_new_expr`表。
  - `func` (INTEGER, 外键): 函数，引用自`function`表。
  - `form` (INTEGER): 形式。
    ```
    case @allocator.form of
        0 = plain
      | 1 = alignment
      ;
    ```
- **示例**:
  ```sql
  CREATE TABLE expr_allocator (
      expr INTEGER PRIMARY KEY,
      func INTEGER,
      form INTEGER,
      FOREIGN KEY (func) REFERENCES function(id)
  );
  ```

### 209. **expr_deallocator**
- **描述**: 表达式释放器。
- **属性**:
  - `expr` (INTEGER, 主键): 表达式，引用自`new_or_delete_expr`表。
  - `func` (INTEGER, 外键): 函数，引用自`function`表。
  - `form` (INTEGER): 形式。
    ```
    case @deallocator.form of
        0 = plain
      | 1 = size
      | 2 = alignment
      | 4 = destroying_delete
      ;
    ```
- **示例**:
  ```sql
  CREATE TABLE expr_deallocator (
      expr INTEGER PRIMARY KEY,
      func INTEGER,
      form INTEGER,
      FOREIGN KEY (func) REFERENCES function(id)
  );
  ```

### 210. **expr_cond_two_operand**
- **描述**: 条件表达式双操作数。
- **属性**:
  - `cond` (INTEGER, 主键): 条件表达式，引用自`conditionalexpr`表。
- **示例**:
  ```sql
  CREATE TABLE expr_cond_two_operand (
      cond INTEGER PRIMARY KEY,
      FOREIGN KEY (cond) REFERENCES conditionalexpr(id)
  );
  ```

### 211. **expr_cond_guard**
- **描述**: 条件表达式守卫。
- **属性**:
  - `cond` (INTEGER, 主键): 条件表达式，引用自`conditionalexpr`表。
  - `guard` (INTEGER, 外键): 守卫，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE expr_cond_guard (
      cond INTEGER,
      guard INTEGER,
      PRIMARY KEY (cond, guard),
      FOREIGN KEY (cond) REFERENCES conditionalexpr(id),
      FOREIGN KEY (guard) REFERENCES expr(id)
  );
  ```

### 212. **expr_cond_true**
- **描述**: 条件表达式真值。
- **属性**:
  - `cond` (INTEGER, 主键): 条件表达式，引用自`conditionalexpr`表。
  - `true` (INTEGER, 外键): 真值，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE expr_cond_true (
      cond INTEGER,
      true INTEGER,
      PRIMARY KEY (cond, true),
      FOREIGN KEY (cond) REFERENCES conditionalexpr(id),
      FOREIGN KEY (true) REFERENCES expr(id)
  );
  ```

### 213. **expr_cond_false**
- **描述**: 条件表达式假值。
- **属性**:
  - `cond` (INTEGER, 主键): 条件表达式，引用自`conditionalexpr`表。
  - `false` (INTEGER, 外键): 假值，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE expr_cond_false (
      cond INTEGER,
      false INTEGER,
      PRIMARY KEY (cond, false),
      FOREIGN KEY (cond) REFERENCES conditionalexpr(id),
      FOREIGN KEY (false) REFERENCES expr(id)
  );
  ```

### 214. **values**
- **描述**: 值。
- **属性**:
  - `id` (INTEGER, 主键): 唯一标识符，引用自`value`。
  - `str` (TEXT): 字符串表示
- **示例**:
  ```sql
  CREATE TABLE values (
      id INTEGER PRIMARY KEY,
      str TEXT,
      FOREIGN KEY (id) REFERENCES value(id)
  );
  ```

### 215. **valuetext**
- **描述**: 值文本。
- **属性**:
  - `id` (INTEGER, 主键): 唯一标识符，引用自`value`。
  - `text` (TEXT): 文本内容
- **示例**:
  ```sql
  CREATE TABLE valuetext (
      id INTEGER PRIMARY KEY,
      text TEXT,
      FOREIGN KEY (id) REFERENCES value(id)
  );
  ```

### 216. **valuebind**
- **描述**: 值绑定。
- **属性**:
  - `val` (INTEGER, 外键): 值，引用自`value`表。
  - `expr` (INTEGER, 外键): 表达式，引用自`expr`表。
- **示例**:
  ```sql
  CREATE TABLE valuebind (
      val INTEGER,
      expr INTEGER,
      PRIMARY KEY (val, expr),
      FOREIGN KEY (val) REFERENCES value(id),
      FOREIGN KEY (expr) REFERENCES expr(id)
  );
  ```

### 217. **fieldoffsets**
- **描述**: 字段偏移量。
- **属性**:
  - `id` (INTEGER, 主键): 唯一标识符，引用自`variable`。
  - `byteoffset` (INTEGER): 字节偏移量。
  - `bitoffset` (INTEGER): 位偏移量。
- **示例**:
  ```sql
  CREATE TABLE fieldoffsets (
      id INTEGER PRIMARY KEY,
      byteoffset INTEGER,
      bitoffset INTEGER,
      FOREIGN KEY (id) REFERENCES variable(id)
  );
  ```

### 218. **bitfield**
- **描述**: 位字段。
- **属性**:
  - `id` (INTEGER, 主键): 唯一标识符，引用自`variable`。
  - `bits` (INTEGER): 位数。
  - `declared_bits` (INTEGER): 声明的位数。
- **示例**:
  ```sql
  CREATE TABLE bitfield (
      id INTEGER PRIMARY KEY,
      bits INTEGER,
      declared_bits INTEGER,
      FOREIGN KEY (id) REFERENCES variable(id)
  );
  ```

**CodeQL's TODO**

```
/* TODO
memberprefix(
    int member: @expr ref,
    int prefix: @expr ref
);
*/

/*
   kind(1) = mbrcallexpr
   kind(2) = mbrptrcallexpr
   kind(3) = mbrptrmbrcallexpr
   kind(4) = ptrmbrptrmbrcallexpr
   kind(5) = mbrreadexpr // x.y
   kind(6) = mbrptrreadexpr // p->y
   kind(7) = mbrptrmbrreadexpr // x.*pm
   kind(8) = mbrptrmbrptrreadexpr // x->*pm
   kind(9) = staticmbrreadexpr // static x.y
   kind(10) = staticmbrptrreadexpr // static p->y
*/
/* TODO
memberaccess(
    int member: @expr ref,
    int kind: int ref
);
*/
```

### 219. **initialisers**
- **描述**: 初始化器。
- **属性**:
  - `init` (INTEGER, 主键): 初始化器，引用自`initialiser`表。
  - `var` (INTEGER, 外键): 变量，引用自`accessible`表。
  - `expr` (INTEGER, 外键): 表达式，引用自`expr`表。
  - `location` (INTEGER, 外键): 位置，引用自`location_expr`表。
- **示例**:
  ```sql
  CREATE TABLE initialisers (
      init INTEGER PRIMARY KEY,
      var INTEGER,
      expr INTEGER,
      location INTEGER,
      FOREIGN KEY (var) REFERENCES accessible(id),
      FOREIGN KEY (expr) REFERENCES expr(id),
      FOREIGN KEY (location) REFERENCES location_expr(id)
  );
  ```

### 220. **braced_initialisers**
- **描述**: 大括号初始化器。
- **属性**:
  - `init` (INTEGER, 主键): 初始化器，引用自`initialiser`表。
- **示例**:
  ```sql
  CREATE TABLE braced_initialisers (
      init INTEGER PRIMARY KEY,
      FOREIGN KEY (init) REFERENCES initialiser(id)
  );
  ```

### 221. **expr_ancestor**
- **描述**: 表达式祖先。
- **属性**:
  - `exp` (INTEGER, 外键): 表达式，引用自`expr`表。
  - `ancestor` (INTEGER, 外键): 祖先，引用自`element`表。
- **示例**:
  ```sql
  CREATE TABLE expr_ancestor (
      exp INTEGER,
      ancestor INTEGER,
      PRIMARY KEY (exp, ancestor),
      FOREIGN KEY (exp) REFERENCES expr(id),
      FOREIGN KEY (ancestor) REFERENCES element(id)
  );
  ```

### 222. **exprs** :DONE
- **描述**: 表达式。
- **属性**:
  - `id` (INTEGER, 主键): 唯一标识符，引用自`expr`。
  - `kind` (INTEGER): 类型。
    ```
    case @expr.kind of
        1 = @errorexpr
    |   2 = @address_of // & AddressOfExpr
    |   3 = @reference_to // ReferenceToExpr (implicit?)
    |   4 = @indirect // * PointerDereferenceExpr
    |   5 = @ref_indirect // ReferenceDereferenceExpr (implicit?)
    // ...
    |   8 = @array_to_pointer // (???)
    |   9 = @vacuous_destructor_call // VacuousDestructorCall
    // ...
    |  11 = @assume // Microsoft
    |  12 = @parexpr
    |  13 = @arithnegexpr
    |  14 = @unaryplusexpr
    |  15 = @complementexpr
    |  16 = @notexpr
    |  17 = @conjugation // GNU ~ operator
    |  18 = @realpartexpr // GNU __real
    |  19 = @imagpartexpr // GNU __imag
    |  20 = @postincrexpr
    |  21 = @postdecrexpr
    |  22 = @preincrexpr
    |  23 = @predecrexpr
    |  24 = @conditionalexpr
    |  25 = @addexpr
    |  26 = @subexpr
    |  27 = @mulexpr
    |  28 = @divexpr
    |  29 = @remexpr
    |  30 = @jmulexpr // C99 mul imaginary
    |  31 = @jdivexpr // C99 div imaginary
    |  32 = @fjaddexpr // C99 add real + imaginary
    |  33 = @jfaddexpr // C99 add imaginary + real
    |  34 = @fjsubexpr // C99 sub real - imaginary
    |  35 = @jfsubexpr // C99 sub imaginary - real
    |  36 = @paddexpr // pointer add (pointer + int or int + pointer)
    |  37 = @psubexpr // pointer sub (pointer - integer)
    |  38 = @pdiffexpr // difference between two pointers
    |  39 = @lshiftexpr
    |  40 = @rshiftexpr
    |  41 = @andexpr
    |  42 = @orexpr
    |  43 = @xorexpr
    |  44 = @eqexpr
    |  45 = @neexpr
    |  46 = @gtexpr
    |  47 = @ltexpr
    |  48 = @geexpr
    |  49 = @leexpr
    |  50 = @minexpr // GNU minimum
    |  51 = @maxexpr // GNU maximum
    |  52 = @assignexpr
    |  53 = @assignaddexpr
    |  54 = @assignsubexpr
    |  55 = @assignmulexpr
    |  56 = @assigndivexpr
    |  57 = @assignremexpr
    |  58 = @assignlshiftexpr
    |  59 = @assignrshiftexpr
    |  60 = @assignandexpr
    |  61 = @assignorexpr
    |  62 = @assignxorexpr
    |  63 = @assignpaddexpr // assign pointer add
    |  64 = @assignpsubexpr // assign pointer sub
    |  65 = @andlogicalexpr
    |  66 = @orlogicalexpr
    |  67 = @commaexpr
    |  68 = @subscriptexpr // access to member of an array, e.g., a[5]
    // ...  69 @objc_subscriptexpr deprecated
    // ...  70 @cmdaccess deprecated
    // ...
    |  73 = @virtfunptrexpr
    |  74 = @callexpr
    // ...  75 @msgexpr_normal deprecated
    // ...  76 @msgexpr_super deprecated
    // ...  77 @atselectorexpr deprecated
    // ...  78 @atprotocolexpr deprecated
    |  79 = @vastartexpr
    |  80 = @vaargexpr
    |  81 = @vaendexpr
    |  82 = @vacopyexpr
    // ...  83 @atencodeexpr deprecated
    |  84 = @varaccess
    |  85 = @thisaccess
    // ...  86 @objc_box_expr deprecated
    |  87 = @new_expr
    |  88 = @delete_expr
    |  89 = @throw_expr
    |  90 = @condition_decl // a variable declared in a condition, e.g., if(int x = y > 2)
    |  91 = @braced_init_list
    |  92 = @type_id
    |  93 = @runtime_sizeof
    |  94 = @runtime_alignof
    |  95 = @sizeof_pack
    |  96 = @expr_stmt // GNU extension
    |  97 = @routineexpr
    |  98 = @type_operand // used to access a type in certain contexts (haven't found any examples yet....)
    |  99 = @offsetofexpr // offsetof ::= type and field
    | 100 = @hasassignexpr // __has_assign ::= type
    | 101 = @hascopyexpr // __has_copy ::= type
    | 102 = @hasnothrowassign // __has_nothrow_assign ::= type
    | 103 = @hasnothrowconstr // __has_nothrow_constructor ::= type
    | 104 = @hasnothrowcopy // __has_nothrow_copy ::= type
    | 105 = @hastrivialassign // __has_trivial_assign ::= type
    | 106 = @hastrivialconstr // __has_trivial_constructor ::= type
    | 107 = @hastrivialcopy // __has_trivial_copy ::= type
    | 108 = @hasuserdestr // __has_user_destructor ::= type
    | 109 = @hasvirtualdestr // __has_virtual_destructor ::= type
    | 110 = @isabstractexpr // __is_abstract ::= type
    | 111 = @isbaseofexpr // __is_base_of ::= type type
    | 112 = @isclassexpr // __is_class ::= type
    | 113 = @isconvtoexpr // __is_convertible_to ::= type type
    | 114 = @isemptyexpr // __is_empty ::= type
    | 115 = @isenumexpr // __is_enum ::= type
    | 116 = @ispodexpr // __is_pod ::= type
    | 117 = @ispolyexpr // __is_polymorphic ::= type
    | 118 = @isunionexpr // __is_union ::= type
    | 119 = @typescompexpr // GNU __builtin_types_compatible ::= type type
    | 120 = @intaddrexpr // frontend internal builtin, used to implement offsetof
    // ...
    | 122 = @hastrivialdestructor // __has_trivial_destructor ::= type
    | 123 = @literal
    | 124 = @uuidof
    | 127 = @aggregateliteral
    | 128 = @delete_array_expr
    | 129 = @new_array_expr
    // ... 130 @objc_array_literal deprecated
    // ... 131 @objc_dictionary_literal deprecated
    | 132 = @foldexpr
    // ...
    | 200 = @ctordirectinit
    | 201 = @ctorvirtualinit
    | 202 = @ctorfieldinit
    | 203 = @ctordelegatinginit
    | 204 = @dtordirectdestruct
    | 205 = @dtorvirtualdestruct
    | 206 = @dtorfielddestruct
    // ...
    | 210 = @static_cast
    | 211 = @reinterpret_cast
    | 212 = @const_cast
    | 213 = @dynamic_cast
    | 214 = @c_style_cast
    | 215 = @lambdaexpr
    | 216 = @param_ref
    | 217 = @noopexpr
    // ...
    | 294 = @istriviallyconstructibleexpr
    | 295 = @isdestructibleexpr
    | 296 = @isnothrowdestructibleexpr
    | 297 = @istriviallydestructibleexpr
    | 298 = @istriviallyassignableexpr
    | 299 = @isnothrowassignableexpr
    | 300 = @istrivialexpr
    | 301 = @isstandardlayoutexpr
    | 302 = @istriviallycopyableexpr
    | 303 = @isliteraltypeexpr
    | 304 = @hastrivialmoveconstructorexpr
    | 305 = @hastrivialmoveassignexpr
    | 306 = @hasnothrowmoveassignexpr
    | 307 = @isconstructibleexpr
    | 308 = @isnothrowconstructibleexpr
    | 309 = @hasfinalizerexpr
    | 310 = @isdelegateexpr
    | 311 = @isinterfaceclassexpr
    | 312 = @isrefarrayexpr
    | 313 = @isrefclassexpr
    | 314 = @issealedexpr
    | 315 = @issimplevalueclassexpr
    | 316 = @isvalueclassexpr
    | 317 = @isfinalexpr
    | 319 = @noexceptexpr
    | 320 = @builtinshufflevector
    | 321 = @builtinchooseexpr
    | 322 = @builtinaddressof
    | 323 = @vec_fill
    | 324 = @builtinconvertvector
    | 325 = @builtincomplex
    | 326 = @spaceshipexpr
    | 327 = @co_await
    | 328 = @co_yield
    | 329 = @temp_init
    | 330 = @isassignable
    | 331 = @isaggregate
    | 332 = @hasuniqueobjectrepresentations
    | 333 = @builtinbitcast
    | 334 = @builtinshuffle
    | 335 = @blockassignexpr
    | 336 = @issame
    | 337 = @isfunction
    | 338 = @islayoutcompatible
    | 339 = @ispointerinterconvertiblebaseof
    | 340 = @isarray
    | 341 = @arrayrank
    | 342 = @arrayextent
    | 343 = @isarithmetic
    | 344 = @iscompletetype
    | 345 = @iscompound
    | 346 = @isconst
    | 347 = @isfloatingpoint
    | 348 = @isfundamental
    | 349 = @isintegral
    | 350 = @islvaluereference
    | 351 = @ismemberfunctionpointer
    | 352 = @ismemberobjectpointer
    | 353 = @ismemberpointer
    | 354 = @isobject
    | 355 = @ispointer
    | 356 = @isreference
    | 357 = @isrvaluereference
    | 358 = @isscalar
    | 359 = @issigned
    | 360 = @isunsigned
    | 361 = @isvoid
    | 362 = @isvolatile
    | 363 = @reuseexpr
    | 364 = @istriviallycopyassignable
    | 365 = @isassignablenopreconditioncheck
    | 366 = @referencebindstotemporary
    | 367 = @issameas
    | 368 = @builtinhasattribute
    | 369 = @ispointerinterconvertiblewithclass
    | 370 = @builtinispointerinterconvertiblewithclass
    | 371 = @iscorrespondingmember
    | 372 = @builtiniscorrespondingmember
    | 373 = @isboundedarray
    | 374 = @isunboundedarray
    | 375 = @isreferenceable
    | 378 = @isnothrowconvertible
    | 379 = @referenceconstructsfromtemporary
    | 380 = @referenceconvertsfromtemporary
    | 381 = @isconvertible
    | 382 = @isvalidwinrttype
    | 383 = @iswinclass
    | 384 = @iswininterface
    | 385 = @istriviallyequalitycomparable
    | 386 = @isscopedenum
    | 387 = @istriviallyrelocatable
    | 388 = @datasizeof
    | 389 = @c11_generic
    | 390 = @requires_expr
    | 391 = @nested_requirement
    | 392 = @compound_requirement
    | 393 = @concept_id
    ;
    ```
  - `location` (INTEGER, 外键): 位置，引用自`location_expr`表。
- **示例**:
  ```sql
  CREATE TABLE exprs (
      id INTEGER PRIMARY KEY,
      kind INTEGER,
      location INTEGER,
      FOREIGN KEY (location) REFERENCES location_expr(id)
  );
  ```
### 223. **expr_reuse**
- **描述**: 表达式重用。
- **属性**:
  - `reuse` (INTEGER, 外键): 重用表达式，引用自`expr`表。
  - `original` (INTEGER, 外键): 原始表达式，引用自`expr`表。
  - `value_category` (INTEGER): 值类别。
- **示例**:
  ```sql
  CREATE TABLE expr_reuse (
      reuse INTEGER,
      original INTEGER,
      value_category INTEGER,
      PRIMARY KEY (reuse, original),
      FOREIGN KEY (reuse) REFERENCES expr(id),
      FOREIGN KEY (original) REFERENCES expr(id)
  );
  ```

### 224. **expr_types**
- **描述**: 表达式类型。
- **属性**:
  - `id` (INTEGER, 外键): 表达式，引用自`expr`表。
  - `typeid` (INTEGER, 外键): 类型，引用自`type`表。
  - `value_category` (INTEGER): 值类别。
- **示例**:
  ```sql
  CREATE TABLE expr_types (
      id INTEGER,
      typeid INTEGER,
      value_category INTEGER,
      PRIMARY KEY (id, typeid),
      FOREIGN KEY (id) REFERENCES expr(id),
      FOREIGN KEY (typeid) REFERENCES type(id)
  );
  ```

### 225. **var_args_expr**
```
@var_args_expr = @vastartexpr +-+
               | @vaendexpr
               | @vaargexpr
               | @vacopyexpr
               ;
```

### 226. **builtin_op**
```
@builtin_op = @var_args_expr +-+
            | @noopexpr
            | @offsetofexpr
            | @intaddrexpr
            | @hasassignexpr
            | @hascopyexpr
            | @hasnothrowassign
            | @hasnothrowconstr
            | @hasnothrowcopy
            | @hastrivialassign
            | @hastrivialconstr
            | @hastrivialcopy
            | @hastrivialdestructor
            | @hasuserdestr
            | @hasvirtualdestr
            | @isabstractexpr
            | @isbaseofexpr
            | @isclassexpr
            | @isconvtoexpr
            | @isemptyexpr
            | @isenumexpr
            | @ispodexpr
            | @ispolyexpr
            | @isunionexpr
            | @typescompexpr
            | @builtinshufflevector
            | @builtinconvertvector
            | @builtinaddressof
            | @istriviallyconstructibleexpr
            | @isdestructibleexpr
            | @isnothrowdestructibleexpr
            | @istriviallydestructibleexpr
            | @istriviallyassignableexpr
            | @isnothrowassignableexpr
            | @istrivialexpr
            | @isstandardlayoutexpr
            | @istriviallycopyableexpr
            | @isliteraltypeexpr
            | @hastrivialmoveconstructorexpr
            | @hastrivialmoveassignexpr
            | @hasnothrowmoveassignexpr
            | @isconstructibleexpr
            | @isnothrowconstructibleexpr
            | @hasfinalizerexpr
            | @isdelegateexpr
            | @isinterfaceclassexpr
            | @isrefarrayexpr
            | @isrefclassexpr
            | @issealedexpr
            | @issimplevalueclassexpr
            | @isvalueclassexpr
            | @isfinalexpr
            | @builtinchooseexpr
            | @builtincomplex
            | @isassignable
            | @isaggregate
            | @hasuniqueobjectrepresentations
            | @builtinbitcast
            | @builtinshuffle
            | @issame
            | @isfunction
            | @islayoutcompatible
            | @ispointerinterconvertiblebaseof
            | @isarray
            | @arrayrank
            | @arrayextent
            | @isarithmetic
            | @iscompletetype
            | @iscompound
            | @isconst
            | @isfloatingpoint
            | @isfundamental
            | @isintegral
            | @islvaluereference
            | @ismemberfunctionpointer
            | @ismemberobjectpointer
            | @ismemberpointer
            | @isobject
            | @ispointer
            | @isreference
            | @isrvaluereference
            | @isscalar
            | @issigned
            | @isunsigned
            | @isvoid
            | @isvolatile
            | @istriviallycopyassignable
            | @isassignablenopreconditioncheck
            | @referencebindstotemporary
            | @issameas
            | @builtinhasattribute
            | @ispointerinterconvertiblewithclass
            | @builtinispointerinterconvertiblewithclass
            | @iscorrespondingmember
            | @builtiniscorrespondingmember
            | @isboundedarray
            | @isunboundedarray
            | @isreferenceable
            | @isnothrowconvertible
            | @referenceconstructsfromtemporary
            | @referenceconvertsfromtemporary
            | @isconvertible
            | @isvalidwinrttype
            | @iswinclass
            | @iswininterface
            | @istriviallyequalitycomparable
            | @isscopedenum
            | @istriviallyrelocatable
            ;
```

### 227. **compound_requirement_is_noexcept**

- **描述**: 表示一个复合要求是否为`noexcept`。
- **属性**:
  - `expr` (INTEGER, 主键): 复合要求的唯一标识符，引用自`compound_requirement`表。

```sql
CREATE TABLE compound_requirement_is_noexcept (
    expr INTEGER PRIMARY KEY,
    FOREIGN KEY (expr) REFERENCES compound_requirement(ref)
);
```

### 228. **new_allocated_type**

- **描述**: 表示通过`new`表达式分配的类型。
- **属性**:
  - `expr` (INTEGER, 主键): `new`表达式的唯一标识符，引用自`new_expr`表。
  - `type_id` (INTEGER): 分配类型的唯一标识符，引用自`type`表。

```sql
CREATE TABLE new_allocated_type (
    expr INTEGER PRIMARY KEY,
    type_id INTEGER,
    FOREIGN KEY (expr) REFERENCES new_expr(ref),
    FOREIGN KEY (type_id) REFERENCES type(ref)
);
```

### 229. **new_array_allocated_type**

- **描述**: 表示通过`new`数组表达式分配的类型。
- **属性**:
  - `expr` (INTEGER, 主键): `new`数组表达式的唯一标识符，引用自`new_array_expr`表。
  - `type_id` (INTEGER): 分配类型的唯一标识符，引用自`type`表。

```sql
CREATE TABLE new_array_allocated_type (
    expr INTEGER PRIMARY KEY,
    type_id INTEGER,
    FOREIGN KEY (expr) REFERENCES new_array_expr(ref),
    FOREIGN KEY (type_id) REFERENCES type(ref)
);
```

### 230. **aggregate_field_init**
- **描述**: 聚合字段初始化。
  > The field being initialized by an initializer expression within an aggregate initializer for a class/struct/union. Position is used to sort repeated initializers.
- **属性**:
  - `aggregate` (INTEGER, 外键): 聚合体，引用自`aggregateliteral`表。
  - `initializer` (INTEGER, 外键): 初始化器，引用自`expr`表。
  - `field` (INTEGER, 外键): 字段，引用自`membervariable`表。
  - `position` (INTEGER): 位置。
- **示例**:
  ```sql
  CREATE TABLE aggregate_field_init (
      aggregate INTEGER,
      initializer INTEGER,
      field INTEGER,
      position INTEGER,
      PRIMARY KEY (aggregate, position),
      FOREIGN KEY (aggregate) REFERENCES aggregateliteral(id),
      FOREIGN KEY (initializer) REFERENCES expr(id),
      FOREIGN KEY (field) REFERENCES membervariable(id)
  );
  ```

### 231. **aggregate_array_init**
- **描述**: 聚合数组初始化。
  > The index of the element being initialized by an initializer expression within an aggregate initializer for an array. Position is used to sort repeated initializers.
- **属性**:
  - `aggregate` (INTEGER, 外键): 聚合体，引用自`aggregateliteral`表。
  - `initializer` (INTEGER, 外键): 初始化器，引用自`expr`表。
  - `element_index` (INTEGER): 元素索引。
  - `position` (INTEGER): 位置。
- **示例**:
  ```sql
  CREATE TABLE aggregate_array_init (
      aggregate INTEGER,
      initializer INTEGER,
      element_index INTEGER,
      position INTEGER,
      PRIMARY KEY (aggregate, position),
      FOREIGN KEY (aggregate) REFERENCES aggregateliteral(id),
      FOREIGN KEY (initializer) REFERENCES expr(id)
  );
  ```

### 232. **ctorinit**
```
@ctorinit = @ctordirectinit +-+
          | @ctorvirtualinit
          | @ctorfieldinit
          | @ctordelegatinginit;
```

### 233. **dtordestruct**
```
@dtordestruct = @dtordirectdestruct +-+
              | @dtorvirtualdestruct
              | @dtorfielddestruct;
```

### 234. **condition_decl_bind** :DONE

- **描述**: 表示条件声明的绑定。
- **属性**:
  - `expr` (INTEGER, 主键): 条件的唯一标识符，引用自`condition_decl`表。
  - `decl` (INTEGER, 主键): 声明的唯一标识符，引用自`declaration`表。

```sql
CREATE TABLE condition_decl_bind (
    expr INTEGER,
    decl INTEGER,
    PRIMARY KEY (expr, decl),
    FOREIGN KEY (expr) REFERENCES condition_decl(ref),
    FOREIGN KEY (decl) REFERENCES declaration(ref)
);
```

### 235. **typeid_bind**

- **描述**: 表示类型ID的绑定。
- **属性**:
  - `expr` (INTEGER, 主键): 表达式的唯一标识符，引用自`type_id`表。
  - `type_id` (INTEGER): 类型的唯一标识符，引用自`type`表。

```sql
CREATE TABLE typeid_bind (
    expr INTEGER PRIMARY KEY,
    type_id INTEGER,
    FOREIGN KEY (expr) REFERENCES type_id(ref),
    FOREIGN KEY (type_id) REFERENCES type(ref)
);
```

### 236. **uuidof_bind**

- **描述**: 表示UUID的绑定。
- **属性**:
  - `expr` (INTEGER, 主键): 表达式的唯一标识符，引用自`uuidof`表。
  - `type_id` (INTEGER): 类型的唯一标识符，引用自`type`表。

```sql
CREATE TABLE uuidof_bind (
    expr INTEGER PRIMARY KEY,
    type_id INTEGER,
    FOREIGN KEY (expr) REFERENCES uuidof(ref),
    FOREIGN KEY (type_id) REFERENCES type(ref)
);
```

### 237. **sizeof_or_alignof**
```
@sizeof_or_alignof = @runtime_sizeof | @runtime_alignof | @datasizeof | @sizeof_pack; +-+
```

### 238. **sizeof_bind**

- **描述**: 表示`sizeof`或`alignof`操作符的绑定。
- **属性**:
  - `expr` (INTEGER, 主键): `sizeof`或`alignof`操作符的唯一标识符，引用自`sizeof_or_alignof`表。
  - `type_id` (INTEGER): 类型的唯一标识符，引用自`type`表。

```sql
CREATE TABLE sizeof_bind (
    expr INTEGER PRIMARY KEY,
    type_id INTEGER,
    FOREIGN KEY (expr) REFERENCES sizeof_or_alignof(ref),
    FOREIGN KEY (type_id) REFERENCES type(ref)
);
```

### 239. **code_block**

- **描述**: 表示代码块。
- **属性**:
  - `block` (INTEGER, 主键): 代码块的唯一标识符，引用自`literal`表。
  - `routine` (INTEGER, 主键): 函数的唯一标识符，引用自`function`表。

```sql
CREATE TABLE code_block (
    block INTEGER PRIMARY KEY,
    routine INTEGER,
    FOREIGN KEY (block) REFERENCES literal(ref),
    FOREIGN KEY (routine) REFERENCES function(ref)
);
```

### 240. **lambdas**

- **描述**: 表示Lambda表达式。
- **属性**:
  - `expr` (INTEGER, 主键): Lambda表达式的唯一标识符，引用自`lambdaexpr`表。
  - `default_capture` (VARCHAR): 默认捕获方式。
  - `has_explicit_return_type` (BOOLEAN): 是否显式指定返回类型。

```sql
CREATE TABLE lambdas (
    expr INTEGER PRIMARY KEY,
    default_capture VARCHAR(255),
    has_explicit_return_type BOOLEAN,
    FOREIGN KEY (expr) REFERENCES lambdaexpr(ref)
);
```

### 241. **lambda_capture**

- **描述**: 表示Lambda表达式的捕获。
- **属性**:
  - `id` (INTEGER, 主键): 捕获的唯一标识符。
  - `lambda` (INTEGER): Lambda表达式的唯一标识符，引用自`lambdaexpr`表。
  - `index` (INTEGER): 捕获的索引。
  - `field` (INTEGER): 成员变量的唯一标识符，引用自`membervariable`表。
  - `captured_by_reference` (BOOLEAN): 是否通过引用捕获。
  - `is_implicit` (BOOLEAN): 是否隐式捕获。
  - `location` (INTEGER): 位置的唯一标识符，引用自`location_default`表。

```sql
CREATE TABLE lambda_capture (
    id INTEGER PRIMARY KEY,
    lambda INTEGER,
    index INTEGER,
    field INTEGER,
    captured_by_reference BOOLEAN,
    is_implicit BOOLEAN,
    location INTEGER,
    FOREIGN KEY (lambda) REFERENCES lambdaexpr(ref),
    FOREIGN KEY (field) REFERENCES membervariable(ref),
    FOREIGN KEY (location) REFERENCES location_default(ref)
);
```

### 242. **funbindexpr** :TODO
```
@funbindexpr = @routineexpr +-+
             | @new_expr
             | @delete_expr
             | @delete_array_expr
             | @ctordirectinit
             | @ctorvirtualinit
             | @ctordelegatinginit
             | @dtordirectdestruct
             | @dtorvirtualdestruct;
```

### 243. **varbindexpr** :DONE
```
@varbindexpr = @varaccess | @ctorfieldinit | @dtorfielddestruct; +-+
```

### 244. **addressable**
```
@addressable = @function | @variable; +-+
```

### 245. **aaccessible**
```
@accessible = @addressable | @enumconstant; +-+
```

### 246. **access**
```
@access = @varaccess | @routineexpr; +-+
```

### 247. **fold**

- **描述**: 表示一个折叠操作。
- **属性**:
  - `expr` (INTEGER, 主键): 折叠表达式的唯一标识符，引用自`foldexpr`表。
  - `operator` (VARCHAR): 运算符，引用自`string`表。
  - `is_left_fold` (BOOLEAN): 是否为左折叠。

```sql
CREATE TABLE fold (
    expr INTEGER PRIMARY KEY,
    operator VARCHAR(255),
    is_left_fold BOOLEAN,
    FOREIGN KEY (expr) REFERENCES foldexpr(ref),
    FOREIGN KEY (operator) REFERENCES string(ref)
);
```

### 248. **stmts** :DONE

- **描述**: 表示语句。
- **属性**:
  - `id` (INTEGER, 主键): 语句的唯一标识符，引用自`stmt`表。
  - `kind` (INTEGER): 语句的类型。
    ```
    case @stmt.kind of
        1 = @stmt_expr
    |   2 = @stmt_if
    |   3 = @stmt_while
    |   4 = @stmt_goto
    |   5 = @stmt_label
    |   6 = @stmt_return
    |   7 = @stmt_block
    |   8 = @stmt_end_test_while // do { ... } while ( ... )
    |   9 = @stmt_for
    |  10 = @stmt_switch_case
    |  11 = @stmt_switch
    |  13 = @stmt_asm // "asm" statement or the body of an asm function
    |  15 = @stmt_try_block
    |  16 = @stmt_microsoft_try // Microsoft
    |  17 = @stmt_decl
    |  18 = @stmt_set_vla_size // C99
    |  19 = @stmt_vla_decl // C99
    |  25 = @stmt_assigned_goto // GNU
    |  26 = @stmt_empty
    |  27 = @stmt_continue
    |  28 = @stmt_break
    |  29 = @stmt_range_based_for // C++11
    // ...  30 @stmt_at_autoreleasepool_block deprecated
    // ...  31 @stmt_objc_for_in deprecated
    // ...  32 @stmt_at_synchronized deprecated
    |  33 = @stmt_handler
    // ...  34 @stmt_finally_end deprecated
    |  35 = @stmt_constexpr_if
    |  37 = @stmt_co_return
    |  38 = @stmt_consteval_if
    |  39 = @stmt_not_consteval_if
    ;
    ```
  - `location` (INTEGER): 语句的位置，引用自`location_stmt`表。

```sql
CREATE TABLE stmts (
    id INTEGER PRIMARY KEY,
    kind INTEGER,
    location INTEGER,
    FOREIGN KEY (id) REFERENCES stmt(ref),
    FOREIGN KEY (location) REFERENCES location_stmt(ref)
);
```

### 249. **type_vla**

- **描述**: 表示可变长度数组的类型。
- **属性**:
  - `type_id` (INTEGER, 主键): 类型的唯一标识符，引用自`type`表。
  - `decl` (INTEGER): 声明的标识符，引用自`stmt_vla_decl`表。

```sql
CREATE TABLE type_vla (
    type_id INTEGER PRIMARY KEY,
    decl INTEGER,
    FOREIGN KEY (type_id) REFERENCES type(ref),
    FOREIGN KEY (decl) REFERENCES stmt_vla_decl(ref)
);
```

### 250. **variable_vla**

- **描述**: 表示可变长度数组的变量。
- **属性**:
  - `var` (INTEGER, 主键): 变量的唯一标识符，引用自`variable`表。
  - `decl` (INTEGER): 声明的标识符，引用自`stmt_vla_decl`表。

```sql
CREATE TABLE variable_vla (
    var INTEGER PRIMARY KEY,
    decl INTEGER,
    FOREIGN KEY (var) REFERENCES variable(ref),
    FOREIGN KEY (decl) REFERENCES stmt_vla_decl(ref)
);
```

### 251. **if_initialization** :DONE

- **描述**: 表示`if`语句的初始化部分。
- **属性**:
  - `if_stmt` (INTEGER, 主键): `if`语句的唯一标识符，引用自`stmt_if`表。
  - `init_id` (INTEGER): 初始化语句的标识符，引用自`stmt`表。

```sql
CREATE TABLE if_initialization (
    if_stmt INTEGER PRIMARY KEY,
    init_id INTEGER,
    FOREIGN KEY (if_stmt) REFERENCES stmt_if(ref),
    FOREIGN KEY (init_id) REFERENCES stmt(ref)
);
```

### 252. **if_then** :DONE

- **描述**: 表示`if`语句的`then`部分。
- **属性**:
  - `if_stmt` (INTEGER, 主键): `if`语句的唯一标识符，引用自`stmt_if`表。
  - `then_id` (INTEGER): `then`语句的标识符，引用自`stmt`表。

```sql
CREATE TABLE if_then (
    if_stmt INTEGER PRIMARY KEY,
    then_id INTEGER,
    FOREIGN KEY (if_stmt) REFERENCES stmt_if(ref),
    FOREIGN KEY (then_id) REFERENCES stmt(ref)
);
```

### 253. **if_else** :DONE

- **描述**: 表示`if`语句的`else`部分。
- **属性**:
  - `if_stmt` (INTEGER, 主键): `if`语句的唯一标识符，引用自`stmt_if`表。
  - `else_id` (INTEGER): `else`语句的标识符，引用自`stmt`表。

```sql
CREATE TABLE if_else (
    if_stmt INTEGER PRIMARY KEY,
    else_id INTEGER,
    FOREIGN KEY (if_stmt) REFERENCES stmt_if(ref),
    FOREIGN KEY (else_id) REFERENCES stmt(ref)
);
```

### 254. **constexpr_if_initialization**

- **描述**: 表示`constexpr if`语句的初始化部分。
- **属性**:
  - `constexpr_if_stmt` (INTEGER, 主键): `constexpr if`语句的唯一标识符，引用自`stmt_constexpr_if`表。
  - `init_id` (INTEGER): 初始化语句的标识符，引用自`stmt`表。

```sql
CREATE TABLE constexpr_if_initialization (
    constexpr_if_stmt INTEGER PRIMARY KEY,
    init_id INTEGER,
    FOREIGN KEY (constexpr_if_stmt) REFERENCES stmt_constexpr_if(ref),
    FOREIGN KEY (init_id) REFERENCES stmt(ref)
);
```

### 255. **constexpr_if_then**

- **描述**: 表示`constexpr if`语句的`then`部分。
- **属性**:
  - `constexpr_if_stmt` (INTEGER, 主键): `constexpr if`语句的唯一标识符，引用自`stmt_constexpr_if`表。
  - `then_id` (INTEGER): `then`语句的标识符，引用自`stmt`表。

```sql
CREATE TABLE constexpr_if_then (
    constexpr_if_stmt INTEGER PRIMARY KEY,
    then_id INTEGER,
    FOREIGN KEY (constexpr_if_stmt) REFERENCES stmt_constexpr_if(ref),
    FOREIGN KEY (then_id) REFERENCES stmt(ref)
);
```

### 256. **constexpr_if_else**

- **描述**: 表示`constexpr if`语句的`else`部分。
- **属性**:
  - `constexpr_if_stmt` (INTEGER, 主键): `constexpr if`语句的唯一标识符，引用自`stmt_constexpr_if`表。
  - `else_id` (INTEGER): `else`语句的标识符，引用自`stmt`表。

```sql
CREATE TABLE constexpr_if_else (
    constexpr_if_stmt INTEGER PRIMARY KEY,
    else_id INTEGER,
    FOREIGN KEY (constexpr_if_stmt) REFERENCES stmt_constexpr_if(ref),
    FOREIGN KEY (else_id) REFERENCES stmt(ref)
);
```


### 257. **stmt_consteval_or_not_consteval_if**
```
@stmt_consteval_or_not_consteval_if = @stmt_consteval_if | @stmt_not_consteval_if; +-+
```

### 258. **consteval_if_then**

- **描述**: 表示`consteval if`语句的`then`部分。
- **属性**:
  - `constexpr_if_stmt` (INTEGER, 主键): `consteval if`语句的唯一标识符，引用自`stmt_consteval_or_not_consteval_if`表。
  - `then_id` (INTEGER): `then`部分的语句标识符，引用自`stmt`表。

```sql
CREATE TABLE consteval_if_then (
    constexpr_if_stmt INTEGER PRIMARY KEY,
    then_id INTEGER,
    FOREIGN KEY (constexpr_if_stmt) REFERENCES stmt_consteval_or_not_consteval_if(ref),
    FOREIGN KEY (then_id) REFERENCES stmt(ref)
);
```

### 259. **consteval_if_else**

- **描述**: 表示`consteval if`语句的`else`部分。
- **属性**:
  - `constexpr_if_stmt` (INTEGER, 主键): `consteval if`语句的唯一标识符，引用自`stmt_consteval_or_not_consteval_if`表。
  - `else_id` (INTEGER): `else`部分的语句标识符，引用自`stmt`表。

```sql
CREATE TABLE consteval_if_else (
    constexpr_if_stmt INTEGER PRIMARY KEY,
    else_id INTEGER,
    FOREIGN KEY (constexpr_if_stmt) REFERENCES stmt_consteval_or_not_consteval_if(ref),
    FOREIGN KEY (else_id) REFERENCES stmt(ref)
);
```

### 260. **while_body** :DONE

- **描述**: 表示`while`循环的主体部分。
- **属性**:
  - `while_stmt` (INTEGER, 主键): `while`语句的唯一标识符，引用自`stmt_while`表。
  - `body_id` (INTEGER): 循环主体的语句标识符，引用自`stmt`表。

```sql
CREATE TABLE while_body (
    while_stmt INTEGER PRIMARY KEY,
    body_id INTEGER,
    FOREIGN KEY (while_stmt) REFERENCES stmt_while(ref),
    FOREIGN KEY (body_id) REFERENCES stmt(ref)
);
```

### 261. **do_body** :DONE

- **描述**: 表示`do-while`循环的主体部分。
- **属性**:
  - `do_stmt` (INTEGER, 主键): `do`语句的唯一标识符，引用自`stmt_end_test_while`表。
  - `body_id` (INTEGER): 循环主体的语句标识符，引用自`stmt`表。

```sql
CREATE TABLE do_body (
    do_stmt INTEGER PRIMARY KEY,
    body_id INTEGER,
    FOREIGN KEY (do_stmt) REFERENCES stmt_end_test_while(ref),
    FOREIGN KEY (body_id) REFERENCES stmt(ref)
);
```

### 262. **switch_initialization** :DONE

- **描述**: 表示`switch`语句的初始化部分。
- **属性**:
  - `switch_stmt` (INTEGER, 主键): `switch`语句的唯一标识符，引用自`stmt_switch`表。
  - `init_id` (INTEGER): 初始化语句的标识符，引用自`stmt`表。

```sql
CREATE TABLE switch_initialization (
    switch_stmt INTEGER PRIMARY KEY,
    init_id INTEGER,
    FOREIGN KEY (switch_stmt) REFERENCES stmt_switch(ref),
    FOREIGN KEY (init_id) REFERENCES stmt(ref)
);
```

### 263. **switch_case** :DONE

- **描述**: 表示`switch`语句的`case`部分。
- **属性**:
  - `switch_stmt` (INTEGER): `switch`语句的唯一标识符，引用自`stmt_switch`表。
  - `index` (INTEGER): `case`的索引。
  - `case_id` (INTEGER, 主键): `case`语句的唯一标识符，引用自`stmt_switch_case`表。

```sql
CREATE TABLE switch_case (
    switch_stmt INTEGER,
    index INTEGER,
    case_id INTEGER PRIMARY KEY,
    PRIMARY KEY (switch_stmt, index),
    FOREIGN KEY (switch_stmt) REFERENCES stmt_switch(ref),
    FOREIGN KEY (case_id) REFERENCES stmt_switch_case(ref)
);
```

### 264. **switch_body** :DONE

- **描述**: 表示`switch`语句的主体部分。
- **属性**:
  - `switch_stmt` (INTEGER, 主键): `switch`语句的唯一标识符，引用自`stmt_switch`表。
  - `body_id` (INTEGER): 主体语句的标识符，引用自`stmt`表。

```sql
CREATE TABLE switch_body (
    switch_stmt INTEGER PRIMARY KEY,
    body_id INTEGER,
    FOREIGN KEY (switch_stmt) REFERENCES stmt_switch(ref),
    FOREIGN KEY (body_id) REFERENCES stmt(ref)
);
```

### 265. **stmt_for_or_range_based_for** :DONE
```
@stmt_for_or_range_based_for = @stmt_for +-+
                             | @stmt_range_based_for;
```

### 266. **for_initialization** :DONE

- **描述**: 表示`for`循环的初始化部分。
- **属性**:
  - `for_stmt` (INTEGER, 主键): `for`语句的唯一标识符，引用自`stmt_for_or_range_based_for`表。
  - `init_id` (INTEGER): 初始化语句的唯一标识符，引用自`stmt`表。

```sql
CREATE TABLE for_initialization (
    for_stmt INTEGER PRIMARY KEY,
    init_id INTEGER,
    FOREIGN KEY (for_stmt) REFERENCES stmt_for_or_range_based_for(ref),
    FOREIGN KEY (init_id) REFERENCES stmt(ref)
);
```

### 267. **for_condition** :DONE

- **描述**: 表示`for`循环的条件部分。
- **属性**:
  - `for_stmt` (INTEGER, 主键): `for`语句的唯一标识符，引用自`stmt_for`表。
  - `condition_id` (INTEGER): 条件的唯一标识符，引用自`expr`表。

```sql
CREATE TABLE for_condition (
    for_stmt INTEGER PRIMARY KEY,
    condition_id INTEGER,
    FOREIGN KEY (for_stmt) REFERENCES stmt_for(ref),
    FOREIGN KEY (condition_id) REFERENCES expr(ref)
);
```

### 268. **for_update** :DONE

- **描述**: 表示`for`循环的更新部分。
- **属性**:
  - `for_stmt` (INTEGER, 主键): `for`语句的唯一标识符，引用自`stmt_for`表。
  - `update_id` (INTEGER): 更新语句的唯一标识符，引用自`expr`表。

```sql
CREATE TABLE for_update (
    for_stmt INTEGER PRIMARY KEY,
    update_id INTEGER,
    FOREIGN KEY (for_stmt) REFERENCES stmt_for(ref),
    FOREIGN KEY (update_id) REFERENCES expr(ref)
);
```

### 269. **for_body** :DONE

- **描述**: 表示`for`循环的主体部分。
- **属性**:
  - `for_stmt` (INTEGER, 主键): `for`语句的唯一标识符，引用自`stmt_for`表。
  - `body_id` (INTEGER): 主体语句的唯一标识符，引用自`stmt`表。

```sql
CREATE TABLE for_body (
    for_stmt INTEGER PRIMARY KEY,
    body_id INTEGER,
    FOREIGN KEY (for_stmt) REFERENCES stmt_for(ref),
    FOREIGN KEY (body_id) REFERENCES stmt(ref)
);
```

### 270. **stmtparent**
```
@stmtparent = @stmt | @expr_stmt ; +-+
```

### 271. **stmtparents**

- **描述**: 表示语句的父级关系。
- **属性**:
  - `id` (INTEGER, 主键): 语句的唯一标识符，引用自`stmt`表。
  - `index` (INTEGER): 语句在父级中的索引。
  - `parent` (INTEGER): 父级语句的唯一标识符，引用自`stmtparent`表。

```sql
CREATE TABLE stmtparents (
    id INTEGER,
    index INTEGER,
    parent INTEGER,
    PRIMARY KEY (id),
    FOREIGN KEY (id) REFERENCES stmt(ref),
    FOREIGN KEY (parent) REFERENCES stmtparent(ref)
);
```

### 272. **ishandler**

- **描述**: 表示一个处理器块。
- **属性**:
  - `block` (INTEGER, 主键): 语句块的唯一标识符，引用自`stmt_block`表。

```sql
CREATE TABLE ishandler (
    block INTEGER PRIMARY KEY,
    FOREIGN KEY (block) REFERENCES stmt_block(ref)
);
```

### 273. **cfgnode**
```
@cfgnode = @stmt | @expr | @function | @initialiser ; +-+
```

### 274. **stmt_decl_bind**

- **描述**: 表示语句声明绑定。
- **属性**:
  - `stmt` (INTEGER, 主键): 语句的唯一标识符，引用自`stmt_decl`表。
  - `num` (INTEGER): 绑定编号。
  - `decl` (INTEGER): 声明的唯一标识符，引用自`declaration`表。

```sql
CREATE TABLE stmt_decl_bind (
    stmt INTEGER,
    num INTEGER,
    decl INTEGER,
    PRIMARY KEY (stmt, num),
    FOREIGN KEY (stmt) REFERENCES stmt_decl(ref),
    FOREIGN KEY (decl) REFERENCES declaration(ref)
);
```

### 275. **stmt_decl_entry_bind**

- **描述**: 表示语句声明条目绑定。
- **属性**:
  - `stmt` (INTEGER, 主键): 语句的唯一标识符，引用自`stmt_decl`表。
  - `num` (INTEGER): 绑定编号。
  - `decl_entry` (INTEGER): 条目的唯一标识符，引用自`element`表。

```sql
CREATE TABLE stmt_decl_entry_bind (
    stmt INTEGER,
    num INTEGER,
    decl_entry INTEGER,
    PRIMARY KEY (stmt, num),
    FOREIGN KEY (stmt) REFERENCES stmt_decl(ref),
    FOREIGN KEY (decl_entry) REFERENCES element(ref)
);
```

### 276. **parameterized_element**
```
@parameterized_element = @function | @stmt_block | @requires_expr; +-+
```

### 277. **blockscope**

- **描述**: 表示一个块的作用域。
- **属性**:
  - `block` (INTEGER, 主键): 块的唯一标识符，引用自`stmt_block`表。
  - `enclosing` (INTEGER): 包围该块的作用域的标识符，引用自`parameterized_element`表。

```sql
CREATE TABLE blockscope (
    block INTEGER PRIMARY KEY,
    enclosing INTEGER,
    FOREIGN KEY (block) REFERENCES stmt_block(ref),
    FOREIGN KEY (enclosing) REFERENCES parameterized_element(ref)
);
```

### 278. **jump**
```
@jump = @stmt_goto | @stmt_break | @stmt_continue; +-+
```

### 279. **jumporlabel**
```
@jumporlabel = @jump | @stmt_label | @literal; +-+
```

### 280. **jumpinfo**

- **描述**: 表示跳转信息。
- **属性**:
  - `id` (INTEGER, 主键): 跳转或块的唯一标识符，引用自`jumporlabel`表。
  - `str` (VARCHAR): 相关的字符串。
  - `target` (INTEGER): 跳转目标，引用自`stmt`表。

```sql
CREATE TABLE jumpinfo (
    id INTEGER PRIMARY KEY,
    str VARCHAR(255),
    target INTEGER,
    FOREIGN KEY (id) REFERENCES jumporlabel(ref),
    FOREIGN KEY (target) REFERENCES stmt(ref)
);
```

### 281. **preprocdirects**

- **描述**: 表示预处理指令。
- **属性**:
  - `id` (INTEGER, 主键): 预处理指令的唯一标识符，引用自`preprocdirect`表。
  - `kind` (INTEGER): 预处理指令的类型。
    ```
    case @preprocdirect.kind of
       0 = @ppd_if
    |  1 = @ppd_ifdef
    |  2 = @ppd_ifndef
    |  3 = @ppd_elif
    |  4 = @ppd_else
    |  5 = @ppd_endif
    |  6 = @ppd_plain_include
    |  7 = @ppd_define
    |  8 = @ppd_undef
    |  9 = @ppd_line
    | 10 = @ppd_error
    | 11 = @ppd_pragma
    | 12 = @ppd_objc_import
    | 13 = @ppd_include_next
    | 14 = @ppd_ms_import
    | 15 = @ppd_elifdef
    | 16 = @ppd_elifndef
    | 18 = @ppd_warning
    ;
    ```
  - `location` (INTEGER): 指令的位置，引用自`location_default`表。

```sql
CREATE TABLE preprocdirects (
    id INTEGER PRIMARY KEY,
    kind INTEGER,
    location INTEGER,
    FOREIGN KEY (id) REFERENCES preprocdirect(id),
    FOREIGN KEY (location) REFERENCES location_default(ref)
);
```


### 282. **ppd_include**
```
@ppd_include = @ppd_plain_include | @ppd_objc_import | @ppd_include_next | @ppd_ms_import; +-+
```

### 283. **pdd_branch**
```
@ppd_branch = @ppd_if | @ppd_ifdef | @ppd_ifndef | @ppd_elif | @ppd_elifdef | @ppd_elifndef; +-+
```

### 284. **preprocpair**

- **描述**: 表示预处理指令对。
- **属性**:
  - `begin` (INTEGER, 主键): 预处理分支的引用，引用自`ppd_branch`表。
  - `elseelifend` (INTEGER): 预处理指令的引用，引用自`preprocdirect`表。

```sql
CREATE TABLE preprocpair (
    begin INTEGER PRIMARY KEY,
    elseelifend INTEGER,
    FOREIGN KEY (begin) REFERENCES ppd_branch(ref),
    FOREIGN KEY (elseelifend) REFERENCES preprocdirect(ref)
);
```

### 285. **preproctrue**

- **描述**: 表示预处理真分支。
- **属性**:
  - `branch` (INTEGER, 主键): 预处理分支的引用，引用自`ppd_branch`表。

```sql
CREATE TABLE preproctrue (
    branch INTEGER PRIMARY KEY,
    FOREIGN KEY (branch) REFERENCES ppd_branch(ref)
);
```

### 286. **preprocfalse**

- **描述**: 表示预处理假分支。
- **属性**:
  - `branch` (INTEGER, 主键): 预处理分支的引用，引用自`ppd_branch`表。

```sql
CREATE TABLE preprocfalse (
    branch INTEGER PRIMARY KEY,
    FOREIGN KEY (branch) REFERENCES ppd_branch(ref)
);
```

### 287. **preproctext**

- **描述**: 表示预处理文本。
- **属性**:
  - `id` (INTEGER, 主键): 预处理指令的唯一标识符，引用自`preprocdirect`表。
  - `head` (VARCHAR): 预处理指令的头部。
  - `body` (VARCHAR): 预处理指令的主体。

```sql
CREATE TABLE preproctext (
    id INTEGER PRIMARY KEY,
    head VARCHAR(255),
    body VARCHAR(255),
    FOREIGN KEY (id) REFERENCES preprocdirect(ref)
);
```

### 288. **includes**

- **描述**: 表示包含的文件。
- **属性**:
  - `id` (INTEGER, 主键): 包含指令的唯一标识符，引用自`ppd_include`表。
  - `included` (INTEGER): 被包含文件的引用，引用自`file`表。

```sql
CREATE TABLE includes (
    id INTEGER PRIMARY KEY,
    included INTEGER,
    FOREIGN KEY (id) REFERENCES ppd_include(ref),
    FOREIGN KEY (included) REFERENCES file(ref)
);
```

### 289. **link_targets**

- **描述**: 表示链接目标。
- **属性**:
  - `id` (INTEGER, 主键): 链接目标的唯一标识符，引用自`link_target`表。
  - `binary` (INTEGER): 二进制文件的引用，引用自`file`表。

```sql
CREATE TABLE link_targets (
    id INTEGER PRIMARY KEY,
    binary INTEGER,
    FOREIGN KEY (id) REFERENCES link_target(id),
    FOREIGN KEY (binary) REFERENCES file(ref)
);
```

### 290. **link_parent**

- **描述**: 表示链接父级。
- **属性**:
  - `element` (INTEGER, 主键): 元素的引用，引用自`element`表。
  - `link_target` (INTEGER): 链接目标的引用，引用自`link_target`表。

```sql
CREATE TABLE link_parent (
    element INTEGER PRIMARY KEY,
    link_target INTEGER,
    FOREIGN KEY (element) REFERENCES element(ref),
    FOREIGN KEY (link_target) REFERENCES link_target(ref)
);
```

### 291. **xmlEncoding**

- **描述**: 表示XML文件的编码。
- **属性**:
  - `id` (INTEGER, 主键): 文件的唯一标识符，引用自`file`表。
  - `encoding` (VARCHAR): 编码格式。

```sql
CREATE TABLE xmlEncoding (
    id INTEGER PRIMARY KEY,
    encoding VARCHAR(255),
    FOREIGN KEY (id) REFERENCES file(ref)
);
```

### 292. **xmlDTDs**

- **描述**: 表示XML文档类型定义。
- **属性**:
  - `id` (INTEGER, 主键): DTD的唯一标识符。
  - `root` (VARCHAR): 根元素名称。
  - `publicId` (VARCHAR): 公共标识符。
  - `systemId` (VARCHAR): 系统标识符。
  - `fileid` (INTEGER): 关联文件的引用，引用自`file`表。

```sql
CREATE TABLE xmlDTDs (
    id INTEGER PRIMARY KEY,
    root VARCHAR(255),
    publicId VARCHAR(255),
    systemId VARCHAR(255),
    fileid INTEGER,
    FOREIGN KEY (fileid) REFERENCES file(ref)
);
```

### 293. **xmlElements**

- **描述**: 表示XML元素。
- **属性**:
  - `id` (INTEGER, 主键): 元素的唯一标识符。
  - `name` (VARCHAR): 元素名称。
  - `parentid` (INTEGER): 父元素的引用，引用自`xmlparent`表。
  - `idx` (INTEGER): 元素索引。
  - `fileid` (INTEGER): 关联文件的引用，引用自`file`表。

```sql
CREATE TABLE xmlElements (
    id INTEGER PRIMARY KEY,
    name VARCHAR(255),
    parentid INTEGER,
    idx INTEGER,
    fileid INTEGER,
    FOREIGN KEY (parentid) REFERENCES xmlparent(ref),
    FOREIGN KEY (fileid) REFERENCES file(ref)
);
```

### 294. **xmlAttrs**

- **描述**: 表示XML属性。
- **属性**:
  - `id` (INTEGER, 主键): 属性的唯一标识符。
  - `elementid` (INTEGER): 关联元素的引用，引用自`xmlelement`表。
  - `name` (VARCHAR): 属性名称。
  - `value` (VARCHAR): 属性值。
  - `idx` (INTEGER): 属性索引。
  - `fileid` (INTEGER): 关联文件的引用，引用自`file`表。

```sql
CREATE TABLE xmlAttrs (
    id INTEGER PRIMARY KEY,
    elementid INTEGER,
    name VARCHAR(255),
    value VARCHAR(255),
    idx INTEGER,
    fileid INTEGER,
    FOREIGN KEY (elementid) REFERENCES xmlelement(ref),
    FOREIGN KEY (fileid) REFERENCES file(ref)
);
```

### 295. **xmlNs**

- **描述**: 表示XML命名空间。
- **属性**:
  - `id` (INTEGER, 主键): 命名空间的唯一标识符。
  - `prefixName` (VARCHAR): 前缀名称。
  - `URI` (VARCHAR): 命名空间URI。
  - `fileid` (INTEGER): 关联文件的引用，引用自`file`表。

```sql
CREATE TABLE xmlNs (
    id INTEGER PRIMARY KEY,
    prefixName VARCHAR(255),
    URI VARCHAR(255),
    fileid INTEGER,
    FOREIGN KEY (fileid) REFERENCES file(ref)
);
```

### 296. **xmlHasNs**

- **描述**: 表示XML元素与命名空间的关联。
- **属性**:
  - `elementId` (INTEGER, 主键): 元素的引用，引用自`xmlnamespaceable`表。
  - `nsId` (INTEGER): 命名空间的引用，引用自`xmlnamespace`表。
  - `fileid` (INTEGER): 关联文件的引用，引用自`file`表。

```sql
CREATE TABLE xmlHasNs (
    elementId INTEGER PRIMARY KEY,
    nsId INTEGER,
    fileid INTEGER,
    FOREIGN KEY (elementId) REFERENCES xmlnamespaceable(ref),
    FOREIGN KEY (nsId) REFERENCES xmlnamespace(ref),
    FOREIGN KEY (fileid) REFERENCES file(ref)
);
```

### 297. **xmlComments**

- **描述**: 表示XML注释。
- **属性**:
  - `id` (INTEGER, 主键): 评论的唯一标识符。
  - `text` (VARCHAR): 评论内容。
  - `parentid` (INTEGER): 父元素的引用，引用自`xmlparent`表。
  - `fileid` (INTEGER): 关联文件的引用，引用自`file`表。

```sql
CREATE TABLE xmlComments (
    id INTEGER PRIMARY KEY,
    text VARCHAR(255),
    parentid INTEGER,
    fileid INTEGER,
    FOREIGN KEY (parentid) REFERENCES xmlparent(ref),
    FOREIGN KEY (fileid) REFERENCES file(ref)
);
```

### 298. **xmlChars**

- **描述**: 表示XML字符数据。
- **属性**:
  - `id` (INTEGER, 主键): 字符数据的唯一标识符。
  - `text` (VARCHAR): 字符数据内容。
  - `parentid` (INTEGER): 父元素的引用，引用自`xmlparent`表。
  - `idx` (INTEGER): 字符数据索引。
  - `isCDATA` (INTEGER): 是否为CDATA。
  - `fileid` (INTEGER): 关联文件的引用，引用自`file`表。

```sql
CREATE TABLE xmlChars (
    id INTEGER PRIMARY KEY,
    text VARCHAR(255),
    parentid INTEGER,
    idx INTEGER,
    isCDATA INTEGER,
    fileid INTEGER,
    FOREIGN KEY (parentid) REFERENCES xmlparent(ref),
    FOREIGN KEY (fileid) REFERENCES file(ref)
);
```

### 299. **xmlparent**
```
@xmlparent = @file | @xmlelement; +-+
```

### 300. **xmlnamespaceable**
```
@xmlnamespaceable = @xmlelement | @xmlattribute; +-+
```

### 301. **xmllocations**

- **描述**: 表示XML元素的位置信息。
- **属性**:
  - `xmlElement` (INTEGER, 主键): XML元素唯一标识符，引用自`xmllocatable`表。
  - `location` (INTEGER, 主键): 位置唯一标识符，引用自`location_default`表。

```sql
CREATE TABLE xmllocations (
    xmlElement INTEGER,
    location INTEGER,
    PRIMARY KEY (xmlElement, location),
    FOREIGN KEY (xmlElement) REFERENCES xmllocatable(ref),
    FOREIGN KEY (location) REFERENCES location_default(ref)
);
```

### 302. **xmllocatable**
```
@xmllocatable = @xmlcharacters +-+
              | @xmlelement
              | @xmlcomment
              | @xmlattribute
              | @xmldtd
              | @file
              | @xmlnamespace;
```
