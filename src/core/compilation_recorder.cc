#include "core/compilation_recorder.h"
#include "model/sql/compilation_args_model.h"
#include "model/sql/compilation_finished_model.h"
#include "model/sql/compilation_model.h"
#include "model/sql/compilation_time_model.h"

CompilationRecorder::CompilationRecorder(AsyncDatabaseManager &db)
    : db_manager_(db), compilation_id_(-1) {}

int CompilationRecorder::createCompilation(
    const std::string &working_directory) {
  auto compilation_model =
      std::make_unique<CompilationModel>(working_directory);
  db_manager_.executeImmediate(std::move(compilation_model->serialize()));
  compilation_id_ = db_manager_.getLastInsertId();
  return compilation_id_;
}

void CompilationRecorder::recordArguments(
    const std::vector<std::string> &flags) {
  int arg_num = 0;
  for (const auto &arg : flags) {
    auto args_model =
        std::make_unique<CompilationArgsModel>(compilation_id_, arg_num++);
    args_model->setArg(arg);
    db_manager_.pushModel(std::move(args_model));
  }
}

void CompilationRecorder::recordTime(CompilationTimeKind kind, double seconds) {
  auto model = std::make_unique<CompilationTimeModel>(compilation_id_,
                                                      time_record_seq_++);
  model->setKind(kind);
  model->setDuration(seconds);
  db_manager_.pushModel(std::move(model));
}

void CompilationRecorder::finalize(double total_cpu, double total_elapsed) {
  auto finished_model =
      std::make_unique<CompilationFinishedModel>(compilation_id_);
  finished_model->setCpuSeconds(total_cpu);
  finished_model->setElapsedSeconds(total_elapsed);
  db_manager_.pushModel(std::move(finished_model));
}