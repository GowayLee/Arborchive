#ifndef _LOCATION_MODEL_H_
#define _LOCATION_MODEL_H_

#include "model/sql/sql_model.h"
#include <cstdint>
#include <string>

class LocationModel : public SQLModel {
public:
  explicit LocationModel(uint32_t type, uint64_t associated_id) {
    setField("id", generateId());
    setField("type", type);
    setField("associated_id", associated_id);
  }

  std::string getTableName() const override { return "location"; }

  std::string serialize() const override {
    return "INSERT INTO " + getTableName() +
           " (id, type, associated_id) VALUES (" + fields_.at("id") + ", " +
           fields_.at("type") + ", " + fields_.at("associated_id") + ")";
  }
};

class LocationDefaultModel : public SQLModel {
public:
  LocationDefaultModel(uint64_t container, uint32_t start_line,
                       uint16_t start_column, uint32_t end_line,
                       uint16_t end_column) {
    setField("id", generateId());
    setField("container", container);
    setField("start_line", start_line);
    setField("start_column", start_column);
    setField("end_line", end_line);
    setField("end_column", end_column);
  }

  std::string getTableName() const override { return "locations_default"; }

  std::string serialize() const override {
    return "INSERT INTO " + getTableName() +
           " (id, container, start_line, start_column, end_line, end_column) "
           "VALUES (" +
           fields_.at("id") + ", " + fields_.at("container") + ", " +
           fields_.at("start_line") + ", " + fields_.at("start_column") + ", " +
           fields_.at("end_line") + ", " + fields_.at("end_column") + ")";
  }
};

class LocationStmtModel : public SQLModel {
public:
  LocationStmtModel(uint64_t container, uint32_t start_line,
                    uint16_t start_column, uint32_t end_line,
                    uint16_t end_column) {
    setField("id", generateId());
    setField("container", container);
    setField("start_line", start_line);
    setField("start_column", start_column);
    setField("end_line", end_line);
    setField("end_column", end_column);
  }

  std::string getTableName() const override { return "locations_stmt"; }

  std::string serialize() const override {
    return "INSERT INTO " + getTableName() +
           " (id, container, start_line, start_column, end_line, "
           "end_column) VALUES (" +
           fields_.at("id") + ", " + fields_.at("container") + ", " +
           fields_.at("start_line") + ", " + fields_.at("start_column") + ", " +
           fields_.at("end_line") + ", " + fields_.at("end_column") + ")";
  }
};

class LocationExprModel : public SQLModel {
public:
  LocationExprModel(uint64_t container, uint32_t start_line,
                    uint16_t start_column, uint32_t end_line,
                    uint16_t end_column) {
    setField("id", generateId());
    setField("container", container);
    setField("start_line", start_line);
    setField("start_column", start_column);
    setField("end_line", end_line);
    setField("end_column", end_column);
  }

  std::string getTableName() const override { return "locations_expr"; }

  std::string serialize() const override {
    return "INSERT INTO " + getTableName() +
           " (id, container, start_line, start_column, end_line, end_column) "
           "VALUES (" +
           fields_.at("id") + ", " + fields_.at("container") + ", " +
           fields_.at("start_line") + ", " + fields_.at("start_column") + ", " +
           fields_.at("end_line") + ", " + fields_.at("end_column") + ")";
  }
};

#endif // _LOCATION_MODEL_H_