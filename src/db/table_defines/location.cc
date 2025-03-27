#include "db/table_defines.h"

std::string createLocationTables() {
  return R"(
    CREATE TABLE IF NOT EXISTS locations (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      associated_id INTEGER NOT NULL,
      type INTEGER NOT NULL
    );

    CREATE TABLE IF NOT EXISTS locations_default (
      id INTEGER PRIMARY KEY,
      container INTEGER,
      start_line INTEGER NOT NULL,
      start_column INTEGER NOT NULL,
      end_line INTEGER NOT NULL,
      end_column INTEGER NOT NULL,
      FOREIGN KEY (id) REFERENCES locations(id),
      FOREIGN KEY (container) REFERENCES container(id)
    );

    CREATE TABLE IF NOT EXISTS locations_stmt (
      id INTEGER PRIMARY KEY,
      container INTEGER,
      start_line INTEGER NOT NULL,
      start_column INTEGER NOT NULL,
      end_line INTEGER NOT NULL,
      end_column INTEGER NOT NULL,
      FOREIGN KEY (id) REFERENCES locations(id),
      FOREIGN KEY (container) REFERENCES container(id)
    );

    CREATE TABLE IF NOT EXISTS locations_expr (
      id INTEGER PRIMARY KEY,
      container INTEGER,
      start_line INTEGER NOT NULL,
      start_column INTEGER NOT NULL,
      end_line INTEGER NOT NULL,
      end_column INTEGER NOT NULL,
      FOREIGN KEY (id) REFERENCES locations(id),
      FOREIGN KEY (container) REFERENCES container(id)
    );
  )";
}

REGISTER_TABLE(LocationTables, createLocationTables);