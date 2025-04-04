#ifndef _CONTAINER_MODEL_H_
#define _CONTAINER_MODEL_H_

#include "model/sql/sql_model.h"
#include <cstdint>

enum class ContainerType { File = 0, Folder = 1 };

class ContainerModel : public SQLModel {
public:
  ContainerModel(ContainerType type, uint64_t associated_id) {
    setField("id", generateId());
    setField("type", static_cast<int>(type));
    setField("associated_id", associated_id);
  }

  std::string getTableName() const override { return "container"; }
};

class FileModel : public SQLModel {
public:
  FileModel(const std::string &name) {
    setField("id", generateId());
    setField("name", name);
  }

  FileModel() = default;

  std::string getTableName() const override { return "files"; }
};

#endif // _CONAINTER_MODEL_H_