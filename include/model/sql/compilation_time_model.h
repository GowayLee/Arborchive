#ifndef _COMPILATION_TIME_MODEL_H_
#define _COMPILATION_TIME_MODEL_H_

#include "model/sql/sql_model.h"

enum class CompilationTimeKind {
  FrontendCpu = 1,
  FrontendElapsed = 2,
  ExtractorCpu = 3,
  ExtractorElapsed = 4
};

class CompilationTimeModel : public SQLModel {
public:
  CompilationTimeModel(int compilation_id, int file_num) {
    setField("id", compilation_id);
    setField("num", file_num);
  }

  std::string getTableName() const override { return "compilation_time"; }

  void setKind(CompilationTimeKind kind) {
    setField("kind", static_cast<int>(kind));
  }

  void setDuration(double seconds) { setField("seconds", seconds); }

  std::string serialize() const override {
    return "INSERT INTO " + getTableName() +
           " (id, num, kind, seconds) VALUES (" + fields_.at("id") + ", " +
           fields_.at("num") + ", " + fields_.at("kind") + ", " +
           fields_.at("seconds") + ")";
  }
};

#endif // _COMPILATION_TIME_MODEL_H_