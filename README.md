# Arborchive

> _Arborchive_ is derived from the Latin word "Arbor" (meaning "tree") and "Archive" , symbolizing the project's core mission of parsing and storing code structure as a tree(AST).

Arborchive 使用 `clang` 解析 C/C++ 代码, 并将AST节点信息存入 `SQLite` 数据库中.

## Dependencies
- [clang](https://clang.llvm.org/)

## Development Environment

当前开发在以下环境中完成：
- **操作系统**: linux-x86_64
- **编译器**: g++ (gcc version 14.2.1)
- **依赖**
  - **clang**: clang version 19.1.7

## Usage
```bash
$ make # 编译
$ make help # 查看帮助信息
$ ./build/demo <Target C/CPP File> # 运行
```

### Example
使用 `./tests/slight.cc` 作为目标文件进行测试
```bash
$ ./build/demo ./tests/slight.cc
```


## Features
- [ ] 解析 C/C++ 代码
  - [x] 记录变量声明&引用节点
  - [x] 记录函数声明&调用节点
  ...
- [ ] 将 AST 节点信息存入 SQLite 数据库中
- [ ] 支持查询结果导出为 CSV 文件
...