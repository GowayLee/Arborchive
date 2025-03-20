#ifndef _COMPILATION_FINISHED_MODEL_H_
#define _COMPILATION_FINISHED_MODEL_H_

#include "model/sql/sql_model.h"

class CompilationFinishedModel : public SQLModel {
public:
  CompilationFinishedModel(int compilation_id) {
    setField("id", compilation_id);
  }

  std::string getTableName() const override { return "compilation_finished"; }

  void setCpuSeconds(double seconds) { setField("cpu_seconds", seconds); }

  void setElapsedSeconds(double seconds) {
    setField("elapsed_seconds", seconds);
  }

  std::string serialize() const override {
    return "INSERT INTO " + getTableName() +
           " (id, cpu_seconds, elapsed_seconds) VALUES (" + fields_.at("id") +
           ", " + fields_.at("cpu_seconds") + ", " +
           fields_.at("elapsed_seconds") + ")";
  }
};

#endif // _COMPILATION_FINISHED_MODEL_H_