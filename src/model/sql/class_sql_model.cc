#include "model/sql/class_model.h"

ClassSQLModel::ClassSQLModel(const std::string &name,
                             const std::string &namespace_) {
  fields_["name"] = name;
  fields_["namespace"] = namespace_;
}

std::string ClassSQLModel::getTableName() const { return "classes"; }

std::string ClassSQLModel::serialize() const {
  return "INSERT INTO classes (name, namespace) VALUES ('" +
         fields_.at("name") + "', '" + fields_.at("namespace") + "')";
}