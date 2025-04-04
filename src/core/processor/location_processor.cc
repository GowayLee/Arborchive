#include "core/processor/location_processor.h"
#include "db/dependency_manager.h"
#include "model/dependency/location_dep.h"
#include "model/sql/location_model.h"
#include "util/logger/macros.h"
#include <clang-c/CXFile.h>
#include <clang-c/Index.h>
#include <filesystem>
#include <iostream>
#include <memory>

void DeclStmtProcessor::handle(CXCursor cursor) { processStatement(cursor); }

void CompoundStmtProcessor::handle(CXCursor cursor) {
  processStatement(cursor);
}

void DeclRefExprProcessor::handle(CXCursor cursor) {
  processExpression(cursor);
}

void CallExprProcessor::handle(CXCursor cursor) { processExpression(cursor); }

void LocationProcessor::processStatement(CXCursor cursor) {
  CXSourceRange range = clang_getCursorExtent(cursor);
  CXSourceLocation start = clang_getRangeStart(range);
  CXSourceLocation end = clang_getRangeEnd(range);
  CXFile file;

  unsigned start_line, start_column;
  clang_getSpellingLocation(start, nullptr, &start_line, &start_column,
                            nullptr);

  unsigned end_line, end_column;
  clang_getSpellingLocation(end, &file, &end_line, &end_column, nullptr);

  auto locStmtModel = std::make_unique<LocationStmtModel>(
      0, start_line, start_column, end_line, end_column);
  auto locModel = std::make_unique<LocationModel>(LocationType::location_stmt,
                                                  locStmtModel->getLastId());

  db_manager_.pushModel(locStmtModel->insert_sql());
  db_manager_.pushModel(locModel->insert_sql());

  // 获取文件名
  CXString filename = clang_getFileName(file);
  std::string filenameStr =
      std::filesystem::path(clang_getCString(filename)).filename().string();

  auto locDep = std::make_unique<LocationDep>(std::move(locStmtModel));
  locDep->setDependency("files", "name", filenameStr);

  DependencyManager::getInstance().addPendingModelDep(
      std::unique_ptr<LocationDep>(std::move(locDep)));

  LOG_DEBUG << "Recorded statement location: " << start_line << ":"
            << start_column << "-" << end_line << ":" << end_column
            << std::endl;

  clang_disposeString(filename);
}

void LocationProcessor::processExpression(CXCursor cursor) {
  CXSourceRange range = clang_getCursorExtent(cursor);
  CXSourceLocation start = clang_getRangeStart(range);
  CXSourceLocation end = clang_getRangeEnd(range);
  CXFile file;

  unsigned start_line, start_column;
  clang_getSpellingLocation(start, nullptr, &start_line, &start_column,
                            nullptr);

  unsigned end_line, end_column;
  clang_getSpellingLocation(end, &file, &end_line, &end_column, nullptr);

  auto locExprModel = std::make_unique<LocationExprModel>(
      0, start_line, start_column, end_line, end_column);
  auto locModel = std::make_unique<LocationModel>(LocationType::location_expr,
                                                  locExprModel->getLastId());

  db_manager_.pushModel(locExprModel->insert_sql()); // Delete ptr locExprModel
  db_manager_.pushModel(locModel->insert_sql());

  // 获取文件名
  CXString filename = clang_getFileName(file);
  std::string filenameStr =
      std::filesystem::path(clang_getCString(filename)).filename().string();

  auto locDep = std::make_unique<LocationDep>(std::move(locExprModel));
  locDep->setDependency("files", "name", filenameStr);

  DependencyManager::getInstance().addPendingModelDep(
      std::unique_ptr<LocationDep>(std::move(locDep)));

  LOG_DEBUG << "Recorded expression location: " << start_line << ":"
            << start_column << "-" << end_line << ":" << end_column
            << std::endl;

  clang_disposeString(filename);
}
