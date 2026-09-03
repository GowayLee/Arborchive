#include "core/router.h"
#include "core/ast_visitor.h"
#include "core/clang_ast_manager.h"
#include "core/compilation_recorder.h"
#include "db/dependency_manager.h"
#include "util/hires_timer.h"
#include "util/logger/macros.h"
#include <filesystem>

bool Router::processCompilation(const Configuration &config) {
  CompRecorder &recorder = CompRecorder::getInstance();

  // 创建编译记录
  recorder.createCompilation(config.compilation.working_directory);

  ClangASTManager &ast_manager = ClangASTManager::getInstance();
  if (!ast_manager.loadConfig(config))
    return false;

  // Persist the same full sequence that configures the actual ClangTool run.
  recorder.recordArguments(ast_manager.getCommandLineArgs());
  recorder.recordBuildMode(1);
  recorder.recordFile(
      std::filesystem::path(config.general.source_path).filename().string());
  recorder.recordVersion();

  HighResTimer frontend_timer;
  frontend_timer.start();

  // 记录前端耗时
  recorder.recordTime(CompTimeKind::FrontendCpu, frontend_timer.cpu_time());
  recorder.recordTime(CompTimeKind::FrontendElapsed, frontend_timer.elapsed());

  // 解析AST
  HighResTimer extractor_timer;
  extractor_timer.start();

  if (!parseAST(config.general.source_path)) {
    LOG_ERROR << "AST parsing failed; dependency resolution and compilation "
                 "finalization were skipped"
              << std::endl;
    return false;
  }

  // Resolve dependencies
  LOG_INFO << "Resolving pending dependencies..." << std::endl;
  DependencyManager::instance().resolveDependencies();
  LOG_INFO << "All dependencies resolved." << std::endl;

  // 记录解析耗时
  recorder.recordTime(CompTimeKind::ExtractorCpu, extractor_timer.cpu_time());
  recorder.recordTime(CompTimeKind::ExtractorElapsed,
                      extractor_timer.elapsed());

  // 完成记录
  recorder.finalize(frontend_timer.cpu_time() + extractor_timer.cpu_time(),
                    frontend_timer.elapsed() + extractor_timer.elapsed());
  return true;
}

bool Router::parseAST(const std::string &source_path) {
  // 使用C++ API处理AST
  return ClangASTManager::getInstance().processAST(
      source_path,
      [this](clang::ASTContext &context) { // 这里定义具体的AST处理逻辑
        // 创建并运行AST访问者
        ASTVisitor visitor(&context);
        visitor.TraverseAST(context);
      });
}
