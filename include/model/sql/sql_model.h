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
  virtual std::string serialize() const = 0;

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
