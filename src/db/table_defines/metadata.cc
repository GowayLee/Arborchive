#include "db/table_defines.h"

std::string createMetaDataTables() {
  return R"(
    CREATE TABLE IF NOT EXISTS externalData (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      path INTEGER,
      column INTEGER,
      value INTEGER
    );

    CREATE TABLE IF NOT EXISTS sourceLocationPrefix (
      prefix INTEGER PRIMARY KEY
    );

    CREATE TABLE IF NOT EXISTS extractor_version (
      codeql_version TEXT,
      frontend_version TEXT
    );
  )";
}

REGISTER_TABLE(MetaDataTables, createMetaDataTables);
