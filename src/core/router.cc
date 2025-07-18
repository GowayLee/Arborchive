#include "core/router.h"
#include "core/ast_visitor.h"
#include "core/clang_ast_manager.h"
#include "core/compilation_recorder.h"
#include "db/dependency_manager.h"
#include "util/hires_timer.h"
#include "util/logger/macros.h"
#include <filesystem>

void Router::processCompilation(const Configuration &config) {
  CompRecorder &recorder = CompRecorder::getInstance();

  // 创建编译记录
  recorder.createCompilation(config.compilation.working_directory);

  // 记录编译参数, 文件名
  recorder.recordArguments(config.compilation.flags);
  recorder.recordFile(
      std::filesystem::path(config.general.source_path).filename().string());

  HighResTimer frontend_timer;
  frontend_timer.start();

  // 使用ClangASTManager处理AST
  ClangASTManager::getInstance().loadConfig(config);

  // 记录前端耗时
  recorder.recordTime(CompTimeKind::FrontendCpu, frontend_timer.cpu_time());
  recorder.recordTime(CompTimeKind::FrontendElapsed, frontend_timer.elapsed());

  // 解析AST
  HighResTimer extractor_timer;
  extractor_timer.start();

  parseAST(config.general.source_path);

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
}

void Router::parseAST(const std::string &source_path) {
  // 使用C++ API处理AST
  ClangASTManager::getInstance().processAST(
      source_path,
      [this](clang::ASTContext &context) { // 这里定义具体的AST处理逻辑
        // 创建并运行AST访问者
        ASTVisitor visitor(context);
        visitor.TraverseDecl(context.getTranslationUnitDecl());
      });
}
