#include "core/router.h"
#include "util/logger/macros.h"

void Router::parseAST(const std::string &filename) {
  // Obtain CXIndex in ClangIndexer
  LOG_DEBUG << "Starting to parse AST for file: " << filename << std::endl;
  auto &clangIndexer = ClangIndexer::getInstance();
  CXTranslationUnit unit = clang_parseTranslationUnit(
      clangIndexer.getIndex(), filename.c_str(),
      clangIndexer.getCommandLineArgs(), clangIndexer.getCommandLineArgsCount(),
      nullptr, 0, 0);
  if (unit) {
    LOG_DEBUG << "Successfully parsed AST for file: " << filename << std::endl;
    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, visitCursor, nullptr);
    clang_disposeTranslationUnit(unit);
    LOG_DEBUG << "Finished processing AST for file: " << filename << std::endl;
  } else {
    LOG_ERROR << "Failed to parse AST for file: " << filename << std::endl;
  }
}

CXChildVisitResult Router::visitCursor(CXCursor cursor, CXCursor parent,
                                       CXClientData client_data) {
  LOG_DEBUG << "Visiting cursor of kind: " << clang_getCursorKind(cursor)
            << std::endl;
  auto handlerIt = BaseProcessor::registry.find(clang_getCursorKind(cursor));
  if (handlerIt != BaseProcessor::registry.end()) {
    auto handler = handlerIt->second();
    handler->handle(cursor); // 调用处理器
    LOG_DEBUG << "Handler successfully processed cursor of kind: "
              << clang_getCursorKind(cursor) << std::endl;
  } else {
    LOG_WARNING << "No handler found for cursor kind: "
                << clang_getCursorKind(cursor) << std::endl;
  }
  return CXChildVisit_Recurse; // 继续遍历子节点
}
