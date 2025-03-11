#ifndef _TABLE_DEFINITIONS_H_
#define _TABLE_DEFINITIONS_H_

#include <functional>
#include <sqlite3.h>
#include <string>
#include <vector>

// 定义表创建函数的类型
using TableCreationFunc = std::function<std::string()>;

// 数据表注册表
class TableRegistry {
public:
  static TableRegistry &instance() {
    static TableRegistry registry;
    return registry;
  }

  void registerTable(const std::string &tableName, TableCreationFunc func) {
    tableCreationFuncs.emplace_back(tableName, func);
  }

  const std::vector<std::pair<std::string, TableCreationFunc>> &
  getTables() const {
    return tableCreationFuncs;
  }

private:
  TableRegistry() = default;
  std::vector<std::pair<std::string, TableCreationFunc>> tableCreationFuncs;
};

// 表定义文件中注册表创建函数Marco
#define REGISTER_TABLE(tableName, func)                                        \
  static bool registerTable##tableName =                                       \
      (TableRegistry::instance().registerTable(#tableName, func), true)

#endif // _TABLE_DEFINITIONS_H_
