#include "core/processor/location_processor.h"
#include "db/storage_facade.h"
#include "model/db/location.h"
#include "util/id_generator.h"
#include "util/logger/macros.h"
#include <clang/Basic/SourceManager.h>
#include <filesystem>
#include <iostream>

using namespace DbModel;

// 处理方法实现
void LocationProcessor::process(const clang::SourceLocation beginLoc,
                                const clang::SourceLocation endLoc) {
  if (!ast_context_) {
    LOG_ERROR << "AST context not set" << std::endl;
    return;
  }

  const auto &sourceManager = ast_context_->getSourceManager();

  if (beginLoc.isInvalid() || endLoc.isInvalid()) {
    LOG_WARNING << "Invalid source location for statement" << std::endl;
    return;
  }

  // 获取开始位置信息
  const unsigned start_line = sourceManager.getSpellingLineNumber(beginLoc);
  const unsigned start_column = sourceManager.getSpellingColumnNumber(beginLoc);

  // 获取结束位置信息
  const unsigned end_line = sourceManager.getSpellingLineNumber(endLoc);
  const unsigned end_column = sourceManager.getSpellingColumnNumber(endLoc);

  // 获取文件信息
  const clang::FileID fileID = sourceManager.getFileID(beginLoc);
  const clang::FileEntry *fileEntry = sourceManager.getFileEntryForID(fileID);
  std::string filename;

  if (fileEntry) {
    // 使用较新Clang版本中通用的方法获取文件名
    llvm::StringRef fileName = sourceManager.getFilename(beginLoc);
    filename = std::filesystem::path(fileName.str()).filename().string();
  } else {
    LOG_WARNING << "Could not determine file for statement" << std::endl;
    return;
  }

  // 创建位置模型
  // FIXME: Currently, only one source file will be parsed, so container_id here
  // is always 0
  LocationStmt locStmtModel = {
      GENID(LocationStmt),          0,
      static_cast<int>(start_line), static_cast<int>(start_column),
      static_cast<int>(end_line),   static_cast<int>(end_column)};
  Location locModel = {GENID(Location), locStmtModel.id};

  // auto locStmtModel = std::make_unique<LocationStmtModel>(
  //     0, start_line, start_column, end_line, end_column);
  // auto locModel =
  // std::make_unique<LocationModel>(LocationType::location_stmt,
  //                                                 locStmtModel->getLastId());

  // // 保存到数据库
  // db_manager_.pushModel(locStmtModel->insert_sql());
  // db_manager_.pushModel(locModel->insert_sql());
  StorageFacade::insertClassObj(locStmtModel);
  StorageFacade::insertClassObj(locModel);

  // 创建依赖关系
  // auto locDep = std::make_unique<LocationDep>(std::move(locStmtModel));
  // locDep->setDependency("files", "name", filename);

  // DependencyManager::getInstance().addPendingModelDep(
  //     std::unique_ptr<LocationDep>(std::move(locDep)));

  LOG_DEBUG << "Recorded statement location: " << start_line << ":"
            << start_column << "-" << end_line << ":" << end_column
            << std::endl;
}
