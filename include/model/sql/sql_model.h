#ifndef _SQL_MODEL_H_
#define _SQL_MODEL_H_

#include "util/id_generator.h"
#include "util/logger/macros.h"
#include <atomic>
#include <map>
#include <mutex>
#include <string>

class SQLModel {
public:
  virtual ~SQLModel() = default;
  SQLModel() = default;
  virtual std::string getTableName() const = 0;

  std::string insert_sql() const {
    if (fields_.empty()) {
      LOG_WARNING << "Table: " << getTableName() << ": No fields to serialize"
                  << std::endl;
      return "";
    }
    // 生成INSERT语句
    std::string fieldList;
    std::string valueList;
    for (const auto &[field, value] : fields_) {
      fieldList += field + ",";
      valueList += value + ",";
    }
    if (!fieldList.empty())
      fieldList.pop_back();
    if (!valueList.empty())
      valueList.pop_back();
    return "INSERT INTO " + getTableName() + " (" + fieldList + ") VALUES (" +
           valueList + ")";
  }

  std::string update_sql() const {
    if (fields_.empty()) {
      LOG_WARNING << "Table: " << getTableName() << ": No fields to serialize"
                  << std::endl;
      return "";
    }
    // 生成UPDATE语句
    std::string update_sql = "UPDATE " + getTableName() + " SET ";
    for (const auto &[field, value] : fields_) {
      if (field != "id") { // 跳过ID字段
        update_sql += field + "=" + value + ",";
      }
    }
    update_sql.pop_back(); // 移除末尾逗号
    update_sql += " WHERE id=" + getField("id");
    return update_sql;
  }

  uint64_t generateId() const {
    return IDGenerator::generateId(getTableName());
  }
  uint64_t getLastId() const {
    return IDGenerator::getLastGeneratedId(getTableName());
  }

  template <typename T> void setField(const std::string &name, const T &value) {
    fields_[name] = std::to_string(value);
  }

  std::string getField(const std::string &name) const {
    auto it = fields_.find(name);
    if (it != fields_.end())
      return it->second;
    LOG_WARNING << "Table: " << getTableName() << ": Field " << name
                << " not found" << std::endl;
    return "";
  }

  // Specialization for string type
  void setField(const std::string &name, const std::string &value) {
    fields_[name] = "'" + value + "'"; // Add quotes for strings
  }

protected:
  std::map<std::string, std::string> fields_;
};

#endif // _SQL_MODEL_H_
