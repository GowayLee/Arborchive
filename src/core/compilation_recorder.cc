#include "core/compilation_recorder.h"
#include "db/storage_facade.h"
#include "model/db/compilation.h"
#include "model/db/container.h"
#include "util/id_generator.h"

using namespace DbModel;

int CompRecorder::createCompilation(const std::string &working_directory) {
  Compilation comp_model = {GENID(Compilation), working_directory};
  STG.insertClassObj(comp_model);
  return compilation_id_ = comp_model.id;
}

void CompRecorder::recordArguments(const std::vector<std::string> &flags) {
  int arg_num = 0;
  for (const auto &arg : flags) {
    CompilationArg comp_arg = {compilation_id_, arg_num++, arg};
    STG.insertClassObj(comp_arg);
  }
}

void CompRecorder::recordTime(CompTimeKind kind, double seconds) {
  CompilationTime comp_time = {compilation_id_, time_record_seq_,
                               static_cast<int>(kind), seconds};
  STG.insertClassObj(comp_time);
}

void CompRecorder::recordFile(const std::string &file) {
  File file_model = {GENID(File), file};
  Container container_model = {GENID(Container), file_model.id,
                               static_cast<int>(ContainerType::File)};
  STG.insertClassObj(file_model);
  STG.insertClassObj(container_model);
}

void CompRecorder::finalize(double total_cpu, double total_elapsed) {
  CompilationFinished finshed_model = {GENID(CompilationFinished), total_cpu,
                                       total_elapsed};
  STG.insertClassObj(finshed_model);
}
