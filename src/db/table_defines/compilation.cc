#include "db/table_defines.h"

std::string createCompilationsTables() {
  return R"(
    CREATE TABLE IF NOT EXISTS compilations (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      cwd TEXT NOT NULL
    );

    CREATE TABLE IF NOT EXISTS compilation_args (
      id INTEGER NOT NULL,
      num INTEGER NOT NULL,
      arg TEXT NOT NULL,
      PRIMARY KEY (id, num),
      FOREIGN KEY (id) REFERENCES compilations(id)
    );

    CREATE TABLE IF NOT EXISTS compilation_build_mode (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      mode INTEGER,
      FOREIGN KEY (id) REFERENCES compilations(id)
    );

    CREATE TABLE IF NOT EXISTS compilation_time (
      id INTEGER NOT NULL,
      num INTEGER NOT NULL,
      kind INTEGER NOT NULL,
      seconds REAL NOT NULL,
      PRIMARY KEY (id, num, kind),
      FOREIGN KEY (id) REFERENCES compilations(id)
    );

    CREATE TABLE IF NOT EXISTS compilation_finished (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      cpu_seconds FLOAT,
      elapsed_seconds FLOAT,
      FOREIGN KEY (id) REFERENCES compilations(id)
    );
  )";
}

REGISTER_TABLE(CompilationsTables, createCompilationsTables);
