# P13 XML 与尾表 deferred 审计

## P13 完成边界

P13 只完成两组具备稳定输入与绑定合同的事实：单 C++ 源编译元数据，以及
Clang 稳定绑定到显式函数声明/定义的 documentation comments。它不再作为
remaining tables 的 catch-all phase。

## 尾表按真实子系统分类

| 子系统 | deferred 内容 | 原因 |
| --- | --- | --- |
| attributes | P7 attribute argument 剩余范围 | 继续由 attribute subsystem 管理，P13 不扩张 P7 |
| control flow | 完整 CFG、`stmtparents`、handler/jump/block scope | 需要独立 CFG/statement phase |
| types | VLA、name qualifier 与 dependent qualifier | 需要 type/name-resolution 合同 |
| build integration | link、SVN、external package/data | 需要构建系统或外部输入协议 |
| diagnostics | diagnostics、`diagnostic_for` | 需要诊断捕获、编号与生命周期合同 |
| annotations | `fileannotations` | 当前没有 `kind`/`name`/`value` 输入协议 |
| comments | 普通未绑定、宏、variable/type/statement 注释绑定 | 当前安全子集只覆盖函数 documentation comments |
| XML | 全部 `xml*` 表 | 当前是单 C++ 源输入，无 XML 输入或 identity/location 合同 |

P7、CFG、VLA、name qualifier、link、diagnostic 等事实必须回到各自 subsystem，
不得为了填充尾表而并入 P13。

## XML deferred 范围

`xmlEncoding`、`xmlDTDs`、`xmlElements`、`xmlAttrs`、`xmlNs`、
`xmlHasNs`、`xmlComments`、`xmlChars` 和 `xmllocations` 全部 deferred。
当前阶段不增加 XML 库、CLI 输入或解析路径。

## 固定重新进入条件

XML 只有在以下条件全部满足后才可重新进入：

1. 明确 C++ 与 XML 多输入协议；
2. 选定 XML parser 与依赖策略；
3. 统一 file/container identity；
4. 定义 parent、namespace、CDATA、DTD 和 location 映射；
5. 单独建立 XML phase，不继续扩张 P13。

任何后续提案都应在独立 XML phase 中同时说明 schema 对齐、迁移影响、fixture
和 SQLite 验收查询。
