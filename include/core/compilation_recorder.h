#ifndef _CORE_COMPILATION_RECORDER_H_
#define _CORE_COMPILATION_RECORDER_H_

#include "db/async_manager.h"
#include "model/sql/compilation_args_model.h"
#include "model/sql/compilation_finished_model.h"
#include "model/sql/compilation_time_model.h"
#include <string>
#include <vector>

class CompilationRecorder {
public:
  CompilationRecorder(AsyncDatabaseManager &db);
  int createCompilation(const std::string &working_directory);

  void recordArguments(const std::vector<std::string> &flags);
  void recordTime(CompilationTimeKind kind, double seconds);
  void finalize(double total_cpu, double total_elapsed);

private:
  AsyncDatabaseManager &db_manager_;
  int compilation_id_;
  int time_record_seq_ = 0;
};

#endif // _CORE_COMPILATION_RECORDER_H_