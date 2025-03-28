#ifndef _CLASS_MODEL_H_
#define _CLASS_MODEL_H_

#include "model/sql/sql_model.h"
#include <map>
#include <string>

class ClassSQLModel : public SQLModel {
private:
  std::map<std::string, std::string> fields_;

public:
  ClassSQLModel(const std::string &name, const std::string &namespace_) {
    fields_["name"] = name;
    fields_["namespace"] = namespace_;
  }

  std::string getTableName() const override { return "classes"; }

  // std::string serialize() const override {
  //   return "INSERT INTO classes (name, namespace) VALUES ('" +
  //          fields_.at("name") + "', '" + fields_.at("namespace") + "')";
  // }
};

#endif // _CLASS_MODEL_H_