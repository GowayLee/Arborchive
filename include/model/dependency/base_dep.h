#ifndef _BASE_DEP_H_
#define _BASE_DEP_H_

#include "model/sql/sql_model.h"
#include <memory>
#include <typeinfo>

class BaseDep {
protected:
  struct DependencyItem {
    std::string table;
    std::string field;
    std::string value;
    std::string original_type;
  };

  std::shared_ptr<SQLModel> sql_model_;
  std::vector<DependencyItem> dependencies_;

public:
  BaseDep(std::shared_ptr<SQLModel> sql_model) : sql_model_(sql_model) {}

  template <typename T>
  void setDependency(const std::string &table, const std::string &field,
                     const T &value) {
    dependencies_.push_back(
        {table, field, std::to_string(value), typeid(T).name()});
  }

  void setDependency(const std::string &table, const std::string &field,
                     const std::string &value) {
    dependencies_.push_back({table, field, "'" + value + "'", "string"});
  }

  std::string getName() const { return sql_model_->getTableName(); }

  virtual bool solve_dependence() const = 0;
};

#endif