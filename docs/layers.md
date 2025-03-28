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

---

**第一阶段：核心基础 (Core Essentials)**


```
1. compilations: 编译信息。编译单元及其配置。
2. compilation_args: 编译参数。传递给编译器的命令行参数。
3. compilation_build_mode: 编译模式(none/manual/auto)?
5. compilation_time: 编译时间。
7. compilation_finished: 编译完成。(CPU Seconds?)
8. externalData: 记录在快照创建期间从CSV文件加载的外部数据。
9. sourceLocationPrefix: 源码位置前缀
16. extractor_version: 提取器版本。
24. files: 文件。记录所有被分析的源文件和头文件。
25. folders: 文件夹。记录源文件和头文件所在的文件夹。
```

```
281. preprocdirects: 预处理指令.
288. includes: 记录文件之间的 #include 关系。
289. link_targets: 链接目标。记录链接的二进制文件(file表)
290. link_parent: 链接父级。
291. xmlEncoding: XML 编码。记录XML文件的编码方式。
292. xmlDTDs: XML DTDs。记录XML文件的DTD信息。
293. xmlElements: XML 元素。记录XML文件的元素信息。
294. xmlAttrs: XML 属性。记录XML文件的属性信息。
295. xmlNs: XML 命名空间。记录XML文件的命名空间信息。
296. xmlHasNs: XML 有命名空间。记录XML文件是否存在命名空间。
297. xmlComments: XML 注释。记录XML文件中的注释信息。
298. xmlChars: XML 字符。记录XML文件中的字符数据。
```

---

**第二阶段：基本语法 (Basic Syntax)**

```
17. @location: 源码位置范围，记录代码元素的位置信息 (x)
18. locations_default: 默认位置信息
19. locations_stmt: 语句位置信息  
20. locations_expr: 表达式位置信息
21. @sourceline: 源码行号定位
23. diagnostics: 编译器诊断信息
26. @container: 容器关系(file/folder)
161. comments: 代码注释信息
170. exprparents: 表达式父子关系
181. varbind: 变量绑定关系
182. funbind: 函数绑定关系  
222. exprs: 基础表达式信息
224. expr_types: 表达式类型信息
248. stmts: 基础语句信息
251. if_initialization: if语句初始化部分
252. if_then: if语句then分支
253. if_else: if语句else分支  
260. while_body: while循环体
265. @stmt_for_or_range_based_for: for循环语句
269. for_body: for循环体
277. blockscope: 块作用域信息
```
