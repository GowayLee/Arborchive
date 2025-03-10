**分层思路**

CodeQL数据库的表结构设计反映了C/C++代码的各个方面，从最基本的词法、语法结构，到更高级的语义信息、控制流、数据流，再到编译、链接、预处理等。因此，我们可以按照以下几个层次来划分：

1.  **核心基础 (Core Essentials):** 文件、目录、基本位置信息、注释等。
2.  **基本语法 (Basic Syntax):** 表达式、语句、类型、变量、函数、命名空间等基本程序构成要素。
3.  **类型系统 (Type System):** 内建类型、派生类型、用户定义类型、类型限定符等。
4.  **函数与作用域 (Functions and Scopes):** 函数定义、参数、调用、重载、作用域、可见性等。
5.  **类与继承 (Classes and Inheritance):** 类定义、成员、继承、虚函数、访问控制等。
6.  **模板 (Templates):** 模板定义、实例化、模板参数、特化等。
7.  **宏 (Macros):** 宏定义、宏展开、宏参数等。
8.  **控制流 (Control Flow):** 条件语句、循环语句、跳转语句、异常处理等。
9.  **编译与链接 (Compilation and Linking):** 编译单元、链接目标、外部数据等。
10. **预处理 (Preprocessing):** 预处理指令、包含关系等。
11. **XML (XML):** 如果适用，包括XML相关的表。
12. **高级特性 (Advanced Features):** 概念(Concepts)、协程(Coroutines)等C++20特性, 以及一些特殊表达式和语句.

**第一阶段：核心基础 (Core Essentials)**

以下是第一阶段应该包含的数据表，这是构建任何C/C++程序分析的基础。

输出格式：`你的输出中的顺序序号 | 原序号. 表名 (中文表名)`

```
1 | 4. compilation_compiling_files (编译文件)
2 | 5. compilation_time (编译时间)
3 | 6. diagnostic_for (诊断信息)
4 | 7. compilation_finished (编译完成)
5 | 8. externalData (外部数据)
6 | 9. sourceLocationPrefix (源位置前缀)
7 | 17. @location (位置)
8 | 18. locations_default (默认位置)
9 | 21. @sourceline (源行)
10 | 22. numlines (行数)
11 | 23. diagnostics (诊断信息)
12 | 24. files (文件)
13 | 25. folders (文件夹)
14 | 161. comments (注释)
15 | 162. commentbinding (注释绑定)
```

**第一阶段包含的数据表说明**

1.  **compilation_compiling_files (编译文件):** 记录正在编译的文件。
2.  **compilation_time (编译时间):** 记录编译时间。
3.  **diagnostic_for (诊断信息):** 记录诊断信息与哪个实体相关。
4.  **compilation_finished (编译完成):** 记录编译是否完成。
5.  **externalData (外部数据):** 存储与外部数据的关联。
6.  **sourceLocationPrefix (源位置前缀):** 存储源文件路径的前缀。
7.  **@location (位置):** 表示代码中的位置。
8.  **locations_default (默认位置):** 存储默认的位置信息。
9.  **@sourceline (源行):** 表示代码所在的行。
10. **numlines (行数):** 存储代码行数。
11. **diagnostics (诊断信息):** 存储编译器的诊断信息（错误、警告等）。
12. **files (文件):** 表示源文件。
13. **folders (文件夹):** 表示文件所在的目录。
14. **comments (注释):** 表示代码中的注释。
15. **commentbinding (注释绑定):** 将注释绑定到代码元素。
