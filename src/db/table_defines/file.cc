#include "db/table_defines.h"

std::string createContainersTables() {
  return R"(
    CREATE TABLE IF NOT EXISTS container (
      id INTEGER PRIMARY KEY,
      associated_id INTEGER NOT NULL,
      type INTEGER NOT NULL
    );

    CREATE TABLE IF NOT EXISTS files (
      id INTEGER PRIMARY KEY,
      name TEXT
    );

    CREATE TABLE IF NOT EXISTS folders (
      id INTEGER PRIMARY KEY,
      name TEXT
    );
  )";
}

REGISTER_TABLE(FilesTables, createContainersTables);
