#include "core/router.h"
#include "core/compilation_recorder.h"
#include "core/processor/base_processor.h"
#include "db/async_manager.h"
#include "db/dependency_manager.h"
#include "interface/clang_indexer.h"
#include "util/hires_timer.h"
#include "util/logger/macros.h"
#include <filesystem>
#include <memory>

void Router::processCompilation(const Configuration &config) {
  AsyncDatabaseManager &dbManager = AsyncDatabaseManager::getInstance();
  CompilationRecorder recorder(dbManager);

  // 创建编译记录
  recorder.createCompilation(config.compilation.working_directory);

  // 记录编译参数, 文件名
  recorder.recordArguments(config.compilation.flags);
  recorder.recordFile(
      std::filesystem::path(config.general.source_path).filename().string());

  HighResTimer frontend_timer;
  frontend_timer.start();

  // Create translation unit
  auto tu = ClangIndexer::getInstance().createTranslationUnit(
      config.general.source_path);

  // 记录前端耗时
  recorder.recordTime(CompilationTimeKind::FrontendCpu,
                      frontend_timer.cpu_time());
  recorder.recordTime(CompilationTimeKind::FrontendElapsed,
                      frontend_timer.elapsed());

  // 解析AST
  HighResTimer extractor_timer;
  extractor_timer.start();
  parseAST(tu);

  // 记录解析耗时
  recorder.recordTime(CompilationTimeKind::ExtractorCpu,
                      extractor_timer.cpu_time());
  recorder.recordTime(CompilationTimeKind::ExtractorElapsed,
                      extractor_timer.elapsed());

  // 完成记录
  recorder.finalize(frontend_timer.cpu_time() + extractor_timer.cpu_time(),
                    frontend_timer.elapsed() + extractor_timer.elapsed());
}

void Router::parseAST(CXTranslationUnit tu) {
  if (!tu) {
    std::cerr << "Invalid translation unit" << std::endl;
    return;
  }

  CXCursor cursor = clang_getTranslationUnitCursor(tu);
  clang_visitChildren(cursor, visitCursor, nullptr);

  auto &dep_manager = DependencyManager::getInstance();
  dep_manager.processPendingModels();
}

CXChildVisitResult Router::visitCursor(CXCursor cursor, CXCursor parent,
                                       CXClientData client_data) {
  (void)parent;
  (void)client_data;
  LOG_DEBUG << "Visiting cursor of kind: " << clang_getCursorKind(cursor)
            << std::endl;
  auto handlerIt = BaseProcessor::registry().find(clang_getCursorKind(cursor));
  if (handlerIt != BaseProcessor::registry().end()) {
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
