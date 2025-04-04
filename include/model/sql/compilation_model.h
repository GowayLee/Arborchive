#ifndef _COMPILATION_MODEL_H_
#define _COMPILATION_MODEL_H_

#include "model/sql/sql_model.h"
#include <string>

class CompilationModel : public SQLModel {
public:
  explicit CompilationModel(const std::string &working_dir) {
    setField("cwd", working_dir);
  }

  std::string getTableName() const override { return "compilations"; }
};

class CompilationArgsModel : public SQLModel {
public:
  CompilationArgsModel(int compilation_id, int num) {
    setField("id", compilation_id);
    setField("num", num);
  }

  CompilationArgsModel() = default;

  std::string getTableName() const override { return "compilation_args"; }

  void setArg(const std::string &arg) { setField("arg", arg); }
};

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

  CompilationTimeModel() = default;

  std::string getTableName() const override { return "compilation_time"; }

  void setKind(CompilationTimeKind kind) {
    setField("kind", static_cast<int>(kind));
  }

  void setDuration(double seconds) { setField("seconds", seconds); }
};

class CompilationFinishedModel : public SQLModel {
public:
  CompilationFinishedModel(int compilation_id) {
    setField("id", compilation_id);
  }

  CompilationFinishedModel() = default;

  std::string getTableName() const override { return "compilation_finished"; }

  void setCpuSeconds(double seconds) { setField("cpu_seconds", seconds); }

  void setElapsedSeconds(double seconds) {
    setField("elapsed_seconds", seconds);
  }
};

#endif // _COMPILATION_MODEL_H_