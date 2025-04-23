#include "core/compilation_recorder.h"
#include "db/storage_facade.h"
#include "model/db/compilation.h"
#include "model/db/container.h"
#include "util/id_generator.h"

// CompRecorder::CompRecorder(AsyncDatabaseManager &db)
//     : db_manager_(db), compilation_id_(-1) {}

using namespace DbModel;

int CompRecorder::createCompilation(const std::string &working_directory) {
  Compilation comp_model = {GENID(Compilation), working_directory};
  StorageFacade::insertClassObj(comp_model);
  return compilation_id_ = comp_model.id;
  // auto compilation_model =
  //     std::make_unique<CompilationModel>(working_directory);
  // db_manager_.executeImmediate(compilation_model->insert_sql());
}

void CompRecorder::recordArguments(const std::vector<std::string> &flags) {
  int arg_num = 0;
  for (const auto &arg : flags) {
    CompilationArg comp_arg = {compilation_id_, arg_num++};
    comp_arg.arg = arg;
    StorageFacade::insertClassObj(comp_arg);
    // auto args_model =
    //     std::make_unique<CompilationArgsModel>(compilation_id_, arg_num++);
  }
}

void CompRecorder::recordTime(CompilationTimeKind kind, double seconds) {
  CompilationTime comp_time = {compilation_id_, time_record_seq_,
                               static_cast<int>(kind), seconds};
  StorageFacade::insertClassObj(comp_time);
  // auto model = std::make_unique<CompilationTimeModel>(compilation_id_,
  //                                                     time_record_seq_++);
  // model->setKind(kind);
  // model->setDuration(seconds);

  // db_manager_.pushModel(model->insert_sql());
}

void CompRecorder::recordFile(const std::string &file) {
  File file_model = {GENID(File), file};
  Container container_model = {GENID(Container), file_model.id,
                               static_cast<int>(ContainerType::File)};
  StorageFacade::insertClassObj(file_model);
  StorageFacade::insertClassObj(container_model);
  // auto fileModel = std::make_unique<FileModel>(file);
  // auto containerModel = std::make_unique<ContainerModel>(
  //     ContainerType::File, fileModel->getLastId());
  // db_manager_.pushModel(fileModel->insert_sql());
  // db_manager_.pushModel(containerModel->insert_sql());
}

void CompRecorder::finalize(double total_cpu, double total_elapsed) {
  CompilationFinished finshed_model = {GENID(CompilationFinished), total_cpu,
                                       total_elapsed};
  StorageFacade::insertClassObj(finshed_model);
  // auto finished_model =
  //     std::make_unique<CompilationFinishedModel>(compilation_id_);
  // finished_model->setCpuSeconds(total_cpu);
  // finished_model->setElapsedSeconds(total_elapsed);
  // db_manager_.pushModel(finished_model->insert_sql());
}
