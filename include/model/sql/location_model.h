#ifndef _LOCATION_MODEL_H_
#define _LOCATION_MODEL_H_

#include "model/sql/sql_model.h"
#include <cstdint>
#include <string>

class LocationModel : public SQLModel {
public:
  explicit LocationModel(uint64_t id, uint32_t type, uint64_t associated_id) {
    setField("id", id);
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
  LocationDefaultModel(uint64_t id, uint64_t container, uint32_t start_line,
                       uint16_t start_column, uint32_t end_line,
                       uint16_t end_column) {
    setField("id", id);
    setField("container", container);
    setField("start_line", start_line);
    setField("start_column", start_column);
    setField("end_line", end_line);
    setField("end_column", end_column);
  }

  std::string getTableName() const override { return "location_default"; }

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
  LocationStmtModel(uint64_t id, uint64_t container,
                    const std::string &stmt_type, uint32_t start_line,
                    uint16_t start_column, uint32_t end_line,
                    uint16_t end_column) {
    setField("id", id);
    setField("container", container);
    setField("stmt_type", "'" + escapeString(stmt_type) + "'");
    setField("start_line", start_line);
    setField("start_column", start_column);
    setField("end_line", end_line);
    setField("end_column", end_column);
  }

  std::string getTableName() const override { return "location_stmt"; }

  std::string serialize() const override {
    return "INSERT INTO " + getTableName() +
           " (id, container, stmt_type, start_line, start_column, end_line, "
           "end_column) VALUES (" +
           fields_.at("id") + ", " + fields_.at("container") + ", " +
           fields_.at("stmt_type") + ", " + fields_.at("start_line") + ", " +
           fields_.at("start_column") + ", " + fields_.at("end_line") + ", " +
           fields_.at("end_column") + ")";
  }

private:
  std::string escapeString(const std::string &str) {
    std::string escaped = str;
    size_t pos = 0;
    while ((pos = escaped.find("'", pos)) != std::string::npos) {
      escaped.replace(pos, 1, "''");
      pos += 2;
    }
    return escaped;
  }
};

class LocationExprModel : public SQLModel {
public:
  LocationExprModel(uint64_t id, uint64_t container,
                    const std::string &expr_type, const std::string &eval_type,
                    uint32_t start_line, uint16_t start_column,
                    uint32_t end_line, uint16_t end_column) {
    setField("id", id);
    setField("container", container);
    setField("expr_type", "'" + escapeString(expr_type) + "'");
    setField("eval_type", "'" + escapeString(eval_type) + "'");
    setField("start_line", start_line);
    setField("start_column", start_column);
    setField("end_line", end_line);
    setField("end_column", end_column);
  }

  std::string getTableName() const override { return "location_expr"; }

  std::string serialize() const override {
    return "INSERT INTO " + getTableName() +
           " (id, container, expr_type, eval_type, start_line, start_column, "
           "end_line, end_column) VALUES (" +
           fields_.at("id") + ", " + fields_.at("container") + ", " +
           fields_.at("expr_type") + ", " + fields_.at("eval_type") + ", " +
           fields_.at("start_line") + ", " + fields_.at("start_column") + ", " +
           fields_.at("end_line") + ", " + fields_.at("end_column") + ")";
  }

private:
  std::string escapeString(const std::string &str) {
    std::string escaped = str;
    size_t pos = 0;
    while ((pos = escaped.find("'", pos)) != std::string::npos) {
      escaped.replace(pos, 1, "''");
      pos += 2;
    }
    return escaped;
  }
};

#endif // _LOCATION_MODEL_H_