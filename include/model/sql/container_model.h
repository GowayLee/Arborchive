#ifndef _CONTAINER_MODEL_H_
#define _CONTAINER_MODEL_H_

#include "model/sql/sql_model.h"

class ContainerModel : public SQLModel {
public:
  ContainerModel(uint32_t type, uint64_t associated_id) {
    setField("id", SQLModel::generateId(getTableName()));
    setField("type", type);
    setField("associated_id", associated_id);
  }

  std::string getTableName() const override { return "container"; }

  std::string serialize() const override {
    return "INSERT INTO " + getTableName() +
           " (id, type, associated_id) VALUES (" + fields_.at("id") + ", " +
           fields_.at("type") + ", " + fields_.at("associated_id") + ")";
  }
};

class FileModel : public SQLModel {
public:
  FileModel(const std::string &name) {
    setField("id", SQLModel::generateId(getTableName()));
    setField("name", name);
  }

  std::string getTableName() const override { return "files"; }

  std::string serialize() const override {
    return "INSERT INTO " + getTableName() + " (id, name) " + "VALUES (" +
           fields_.at("id") + ", " + fields_.at("name") + ")";
  }
};

#endif // _CONAINTER_MODEL_H_