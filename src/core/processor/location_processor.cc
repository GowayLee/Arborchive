#include "core/processor/location_processor.h"
#include "model/sql/location_model.h"
#include "util/logger/macros.h"
#include <clang-c/Index.h>
#include <iostream>
#include <memory>
#include <utility>

// 实现DeclStmtProcessor的handle方法
void DeclStmtProcessor::handle(CXCursor cursor) { processStatement(cursor); }

// 实现CompoundStmtProcessor的handle方法
void CompoundStmtProcessor::handle(CXCursor cursor) {
  processStatement(cursor);
}

// 实现DeclRefExprProcessor的handle方法
void DeclRefExprProcessor::handle(CXCursor cursor) {
  processExpression(cursor);
}

// 实现CallExprProcessor的handle方法
void CallExprProcessor::handle(CXCursor cursor) { processExpression(cursor); }

// 保留原有的LocationProcessor方法实现
void LocationProcessor::processStatement(CXCursor cursor) {
  CXSourceRange range = clang_getCursorExtent(cursor);
  CXSourceLocation start = clang_getRangeStart(range);
  CXSourceLocation end = clang_getRangeEnd(range);

  unsigned start_line, start_column;
  clang_getSpellingLocation(start, nullptr, &start_line, &start_column,
                            nullptr);

  unsigned end_line, end_column;
  clang_getSpellingLocation(end, nullptr, &end_line, &end_column, nullptr);

  uint64_t id = clang_hashCursor(cursor);
  auto model = std::make_unique<LocationStmtModel>(
      id, 0, start_line, start_column, end_line, end_column);
  db_manager_.pushModel(std::move(model));

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

  uint64_t id = clang_hashCursor(cursor);
  auto model = std::make_unique<LocationExprModel>(
      id, 0, start_line, start_column, end_line, end_column);
  db_manager_.pushModel(std::move(model));

  LOG_DEBUG << "Recorded expression location: " << start_line << ":"
            << start_column << "-" << end_line << ":" << end_column
            << std::endl;
}
