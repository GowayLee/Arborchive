#include "core/router.h"
#include "core/processor/base_processor.h"
#include "db/async_manager.h"
#include "interface/clang_indexer.h"
#include "model/sql/compilation_finished_model.h"
#include "model/sql/compilation_model.h"
#include "model/sql/compilation_time_model.h"
#include "util/hires_timer.h"
#include "util/logger/macros.h"
#include <memory>

void Router::processCompilation(const Configuration &config) {
  // 创建编译记录
  AsyncDatabaseManager &dbManager = AsyncDatabaseManager::getInstance();
  auto compilation_model =
      std::make_unique<CompilationModel>(config.compilation.working_directory);
  dbManager.executeImmediate(std::move(compilation_model->serialize()));
  int compilation_id = dbManager.getLastInsertId();

  HighResTimer frontend_timer;
  frontend_timer.start();

  // Create translation unit
  auto tu = ClangIndexer::getInstance().createTranslationUnit(
      config.general.source_path);

  // 记录前端耗时
  auto create_time_record = [&](CompilationTimeKind kind, double seconds) {
    auto model = std::make_unique<CompilationTimeModel>(compilation_id, 0);
    model->setKind(kind);
    model->setDuration(seconds);
    dbManager.pushModel(std::move(model));
  };

  create_time_record(CompilationTimeKind::FrontendCpu,
                     frontend_timer.cpu_time());
  create_time_record(CompilationTimeKind::FrontendElapsed,
                     frontend_timer.elapsed());

  // 解析AST
  HighResTimer extractor_timer;
  extractor_timer.start();
  parseAST(tu);

  // 记录解析耗时
  create_time_record(CompilationTimeKind::ExtractorCpu,
                     extractor_timer.cpu_time());
  create_time_record(CompilationTimeKind::ExtractorElapsed,
                     extractor_timer.elapsed());

  // 记录编译完成信息
  auto finished_model =
      std::make_unique<CompilationFinishedModel>(compilation_id);
  finished_model->setCpuSeconds(frontend_timer.cpu_time() +
                                extractor_timer.cpu_time());
  finished_model->setElapsedSeconds(frontend_timer.elapsed() +
                                    extractor_timer.elapsed());
  dbManager.pushModel(std::move(finished_model));
}

void Router::parseAST(CXTranslationUnit tu) {
  if (!tu) {
    std::cerr << "Invalid translation unit" << std::endl;
    return;
  }

  CXCursor cursor = clang_getTranslationUnitCursor(tu);
  clang_visitChildren(cursor, visitCursor, nullptr);
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
