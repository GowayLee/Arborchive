# Arborchive Local Setup Summary

这份笔记记录这次在 macOS + Antigravity 上配置 Arborchive 的经验。核心结论是：构建依赖和编辑器诊断依赖要分开看。`make` 能过不代表 clangd 一定不报错；clangd 报很多红线也不一定代表项目不能编译。

## 1. 先确认项目需要什么

Arborchive 使用 Clang/LLVM 解析 C/C++ AST，并把结果写入 SQLite。当前代码实际依赖：

- LLVM/Clang 19
- SQLite3
- toml11
- C++17 编译器
- clangd，用于编辑器里的 C++ 诊断和跳转

在 macOS 上不要直接用系统自带的 Clang/clangd。系统 clangd 路径通常是：

```bash
/Library/Developer/CommandLineTools/usr/bin/clangd
```

它不适合这个项目，因为项目需要 Homebrew 安装的 LLVM 19 头文件和 libclang-cpp。

## 2. 安装依赖

推荐用 Homebrew：

```bash
brew install llvm@19 toml11 sqlite
```

检查安装结果：

```bash
/opt/homebrew/opt/llvm@19/bin/llvm-config --version
/opt/homebrew/opt/llvm@19/bin/clangd --version
ls -la /opt/homebrew/include/toml.hpp
```

期望 LLVM 版本是 `19.1.7` 左右，`toml.hpp` 应该存在。

## 3. 配 Makefile

这个项目在 macOS 上需要让 Makefile 优先使用 `llvm@19`，并且链接时不要误用 Homebrew 默认 LLVM 22 的库。

关键配置应该类似：

```make
CXXFLAGS ?= -Wall -Wextra -pedantic -std=c++17 -Iinclude -I/opt/homebrew/include -g

LLVM_CONFIG ?= $(or $(wildcard /opt/homebrew/opt/llvm@19/bin/llvm-config),llvm-config)
LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIBS = $(shell $(LLVM_CONFIG) --libs core)

LDLIBS = $(LLVM_LDFLAGS) -lclang-cpp $(LLVM_LIBS) -lsqlite3
```

链接规则里让 `$(LDLIBS)` 放在 `$(LDFLAGS)` 前面：

```make
$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)
```

原因：如果环境变量里有 `LDFLAGS=-L/opt/homebrew/opt/llvm/lib`，可能会先链接到 LLVM 22 的 `libclang-cpp`，导致和 LLVM 19 头文件/API 不匹配。

另外，`src/util/key_generator/*.cc` 也包含 Clang AST 头文件，需要用 `$(LLVM_CXXFLAGS)` 编译。

## 4. 构建验证

标准验证：

```bash
make clean
make
```

构建成功后会生成：

```bash
build/demo
```

也可以跑示例：

```bash
make run
```

如果只是看到一堆 warning，但最终 exit code 是 0，说明构建成功。warning 多不等于失败。

## 5. toml11 的判断

如果报错是：

```text
toml.hpp file not found
```

才优先怀疑 toml11 没装或者 include 路径没加。

检查：

```bash
brew list toml11
ls -la /opt/homebrew/include/toml.hpp
```

这次的问题不是 toml11。`toml11` 已经装好了，真正导致 Antigravity 里大量红线的是 clangd 没拿到正确 include 路径。

## 6. 配 clangd

项目根目录需要一个 `.clangd`。注意：当前 `.gitignore` 忽略了 `.clangd`，所以别人 clone 项目后不会自动拿到这份配置，需要手动创建。

推荐内容：

```yaml
CompileFlags:
  Add:
    - -xc++
    - -std=c++17
    - -stdlib=libc++
    - -I/Users/y4n9/Workspace/Projects/My-github-projects/Arborchive/include
    - -I/opt/homebrew/include
    - -I/opt/homebrew/Cellar/llvm@19/19.1.7/include
    - -D__STDC_CONSTANT_MACROS
    - -D__STDC_FORMAT_MACROS
    - -D__STDC_LIMIT_MACROS
```

重点：项目 include 路径最好写绝对路径，不要只写：

```yaml
- -Iinclude
```

因为 clangd fallback 编译不同 `.cc` 文件时，工作目录可能变成源文件所在目录，`-Iinclude` 会被解释错，然后出现类似：

```text
interface/config_loader.h file not found
core/processor/variable_processor.h file not found
```

这类报错不是 toml 问题，是项目头文件路径没被 clangd 正确解析。

## 7. 配 Antigravity

Antigravity 用户设置里要指定 Homebrew LLVM 19 的 clangd：

```json
"clangd.path": "/opt/homebrew/opt/llvm@19/bin/clangd",
"C_Cpp.intelliSenseEngine": "disabled"
```

确认当前 clangd 路径：

```bash
ps aux | rg -i 'clangd' | rg -v rg
```

正确结果应该包含：

```bash
/opt/homebrew/opt/llvm@19/bin/clangd
```

如果看到的是：

```bash
/Library/Developer/CommandLineTools/usr/bin/clangd
```

说明 Antigravity 还在用系统 clangd，需要 reload 或重启。

## 8. Reload Antigravity

在 Antigravity 里：

```text
Cmd + Shift + P
Developer: Reload Window
```

或者：

```text
Cmd + Shift + P
clangd: Restart language server
```

也可以终端里杀掉 clangd，让 Antigravity 自动重新拉起：

```bash
pkill -f '/opt/homebrew/opt/llvm@19/bin/clangd'
```

之后再检查：

```bash
ps aux | rg -i 'clangd' | rg -v rg
```

## 9. clangd 验证

用 Homebrew LLVM 19 的 clangd 直接检查文件：

```bash
/opt/homebrew/opt/llvm@19/bin/clangd --check=src/interface/config_loader.cc --enable-config
/opt/homebrew/opt/llvm@19/bin/clangd --check=src/core/processor/variable_processor.cc --enable-config
```

如果 `config_loader.cc` 能到：

```text
All checks completed, 0 errors
```

说明 `toml.hpp` 和项目 include 基本都配置好了。

有时 clangd 会报类似：

```text
tweak: ExtractFunction ==> FAIL
tweak: DefineOutline ==> FAIL
```

这通常是 clangd 的重构辅助功能失败，不是项目 include 或编译失败。

## 10. 快速排错顺序

拿到项目后按这个顺序排：

1. `brew install llvm@19 toml11 sqlite`
2. 确认 `/opt/homebrew/opt/llvm@19/bin/llvm-config --version`
3. 确认 `/opt/homebrew/include/toml.hpp`
4. `make clean && make`
5. 创建或检查 `.clangd`
6. Antigravity 设置 `"clangd.path": "/opt/homebrew/opt/llvm@19/bin/clangd"`
7. `Developer: Reload Window`
8. `ps aux | rg -i 'clangd' | rg -v rg`
9. 用 `clangd --check=... --enable-config` 验证具体文件

## 11. 这次的关键经验

- `make` 失败时，先看是不是 `llvm-config` 找不到，或者链接到了错误版本的 LLVM。
- Homebrew 默认 `llvm` 可能是 22，但这个项目代码更适合 LLVM 19。
- `toml11` 是 header-only，装好后关键文件是 `/opt/homebrew/include/toml.hpp`。
- 编辑器红线多，优先看 clangd 是否使用了正确路径和 `.clangd` 是否生效。
- `.clangd` 里的项目 include 用绝对路径更稳。
- Antigravity reload 后仍有旧红线时，重启 clangd 或重新打开文件。
