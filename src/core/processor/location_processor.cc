#include "core/processor/location_processor.h"
#include "model/sql/location_model.h"
#include "util/logger/macros.h"
#include <clang-c/Index.h>
#include <iostream>
#include <memory>
#include <utility>

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

  unsigned start_line, start_column;
  clang_getSpellingLocation(start, nullptr, &start_line, &start_column,
                            nullptr);

  unsigned end_line, end_column;
  clang_getSpellingLocation(end, nullptr, &end_line, &end_column, nullptr);

  auto locStmtModel = std::make_unique<LocationStmtModel>(
      0, start_line, start_column, end_line, end_column);
  auto locModel = std::make_unique<LocationModel>(LocationType::location_stmt,
                                                  locStmtModel->getLastId());
  db_manager_.pushModel(std::move(locStmtModel));
  db_manager_.pushModel(std::move(locModel));

  LOG_DEBUG << "Recorded statement location: " << start_line << ":"
            << start_column << "-" << end_line << ":" << end_column
            << std::endl;
}

void LocationProcessor::processExpression(CXCursor cursor) {
  CXSourceRange range = clang_getCursorExtent(cursor);
  CXSourceLocation start = clang_getRangeStart(range);
  CXSourceLocation end = clang_getRangeEnd(range);

  unsigned start_line, start_column;
  clang_getSpellingLocation(start, nullptr, &start_line, &start_column,
                            nullptr);

  unsigned end_line, end_column;
  clang_getSpellingLocation(end, nullptr, &end_line, &end_column, nullptr);

  auto locExprModel = std::make_unique<LocationExprModel>(
      0, start_line, start_column, end_line, end_column);
  auto locModel = std::make_unique<LocationModel>(LocationType::location_expr,
                                                  locExprModel->getLastId());
  db_manager_.pushModel(std::move(locExprModel));
  db_manager_.pushModel(std::move(locModel));

  LOG_DEBUG << "Recorded expression location: " << start_line << ":"
            << start_column << "-" << end_line << ":" << end_column
            << std::endl;
}
