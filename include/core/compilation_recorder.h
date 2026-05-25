#ifndef _CORE_COMP_RECORDER_H_
#define _CORE_COMP_RECORDER_H_

#include "model/db/compilation.h"
#include <optional>
#include <string>
#include <vector>

class CompRecorder {
public:
  int createCompilation(const std::string &working_directory);

  void recordArguments(const std::vector<std::string> &flags);
  void recordTime(CompTimeKind kind, double seconds);
  int recordFile(const std::string &file);
  std::optional<int> getSourceFileId() const;
  void finalize(double total_cpu, double total_elapsed);

  static CompRecorder &getInstance() {
    static CompRecorder instance;
    return instance;
  }

  ~CompRecorder() = default;

  CompRecorder(const CompRecorder &) = delete;

  CompRecorder &operator=(const CompRecorder &) = delete;

private:
  CompRecorder() = default;
  int compilation_id_;
  int source_file_id_ = -1;
  int time_record_seq_ = 0;
};

#endif // _CORE_COMP_RECORDER_H_
