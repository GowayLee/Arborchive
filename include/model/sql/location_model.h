#ifndef _LOCATION_MODEL_H_
#define _LOCATION_MODEL_H_

#include "model/sql/sql_model.h"
#include <cstdint>
#include <string>

enum class LocationType {
  location_stmt = 0,
  location_expr = 1,
  location_default = 2
};

class LocationModel : public SQLModel {
public:
  explicit LocationModel(LocationType type, uint64_t associated_id) {
    setField("id", generateId());
    setField("type", static_cast<int>(type));
    setField("associated_id", associated_id);
  }

  std::string getTableName() const override { return "locations"; }
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
};

#endif // _LOCATION_MODEL_H_
