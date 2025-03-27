#include "db/table_defines.h"

std::string createFilesTables() {
  return R"(
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

REGISTER_TABLE(FilesTables, createFilesTables);
