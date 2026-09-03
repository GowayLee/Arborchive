#include "core/compilation_recorder.h"
#include "core/version.h"
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

void CompRecorder::recordBuildMode(int mode) {
  CompilationBuildMode build_mode = {compilation_id_, mode};
  STG.insertClassObj(build_mode);
}

void CompRecorder::recordVersion() {
  ExtractorVersion version = {ArborchiveVersion::codeql_version,
                              ArborchiveVersion::frontend_version};
  STG.insertClassObj(version);
}

void CompRecorder::recordTime(CompTimeKind kind, double seconds) {
  CompilationTime comp_time = {compilation_id_, time_record_seq_,
                               static_cast<int>(kind), seconds};
  STG.insertClassObj(comp_time);
}

int CompRecorder::recordFile(const std::string &file) {
  File file_model = {GENID(File), file};
  Container container_model = {GENID(Container), file_model.id,
                               static_cast<int>(ContainerType::File)};
  STG.insertClassObj(file_model);
  STG.insertClassObj(container_model);
  source_file_id_ = file_model.id;
  CompilationCompilingFile compiling_file = {compilation_id_, 0,
                                             source_file_id_};
  STG.insertClassObj(compiling_file);
  return source_file_id_;
}

std::optional<int> CompRecorder::getSourceFileId() const {
  if (source_file_id_ < 0) {
    return std::nullopt;
  }
  return source_file_id_;
}

void CompRecorder::finalize(double total_cpu, double total_elapsed) {
  CompilationFinished finished_model = {compilation_id_, total_cpu,
                                        total_elapsed};
  STG.insertClassObj(finished_model);
}
