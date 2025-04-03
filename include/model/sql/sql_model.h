#ifndef _SQL_MODEL_H_
#define _SQL_MODEL_H_

#include "util/id_generator.h"
#include "util/logger/macros.h"
#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

class SQLModel {
public:
  virtual ~SQLModel() = default;
  virtual std::string getTableName() const = 0;

  virtual bool try_solve_dependence() { return true; }
  virtual bool checkDependencies() const { return true; }

  std::string serialize() const {
    if (fields_.empty()) {
      LOG_WARNING << "Table: " << getTableName() << ": No fields to serialize"
                  << std::endl;
      return "";
    }
    // 动态构建字段列表和值列表
    std::string fieldList;
    std::string valueList;

    for (const auto &[field, value] : fields_) {
      fieldList += field + ",";
      valueList += value + ",";
    }
    // 去除末尾逗号
    if (!fieldList.empty()) {
      fieldList.pop_back();
    }
    if (!valueList.empty()) {
      valueList.pop_back();
    }
    return "INSERT INTO " + getTableName() + " (" + fieldList + ") VALUES (" +
           valueList + ")";
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

  // Specialization for string type
  void setField(const std::string &name, const std::string &value) {
    fields_[name] = "'" + value + "'"; // Add quotes for strings
  }

protected:
  std::map<std::string, std::string> fields_;
};

#endif // _SQL_MODEL_H_
