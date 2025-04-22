#include "core/clang_ast_manager.h"
#include "util/logger/macros.h"
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <memory>

// 创建自定义的clang组件
class CustomASTConsumer : public clang::ASTConsumer {
public:
  CustomASTConsumer(std::function<void(clang::ASTContext &)> cb,
                    clang::ASTContext &ctx)
      : callback(std::move(cb)), context(ctx) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    callback(Context); // FIXME: ?
  }

private:
  std::function<void(clang::ASTContext &)> callback;
  clang::ASTContext &context;
};

class CustomASTAction : public clang::ASTFrontendAction {
public:
  CustomASTAction(std::function<void(clang::ASTContext &)> cb)
      : callback(std::move(cb)) {}

  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI,
                    llvm::StringRef InFile) override {
    return std::make_unique<CustomASTConsumer>(callback, CI.getASTContext());
  }

private:
  std::function<void(clang::ASTContext &)> callback;
};

struct CustomFrontendActionFactory
    : public clang::tooling::FrontendActionFactory {
  explicit CustomFrontendActionFactory(
      std::function<void(clang::ASTContext &)> cb)
      : clang::tooling::FrontendActionFactory(), callback(cb) {}
  std::unique_ptr<clang::FrontendAction> create() override {
    auto *action =
        dynamic_cast<clang::FrontendAction *>(new CustomASTAction(callback));
    return std::unique_ptr<clang::FrontendAction>(action);
  }

private:
  std::function<void(clang::ASTContext &)> callback;
};

///////////////////////////////////////////////////////////

ClangASTManager::ClangASTManager() {}

bool ClangASTManager::loadConfig(const Configuration &config) {
  // 根据配置文件设置相应的成员变量
  sourcePath = config.general.source_path;
  includePaths = config.compilation.include_paths;
  defines = config.compilation.defines;
  cxxStandard = config.compilation.cxx_standard;
  flags = config.compilation.flags;

  // 转换命令行参数
  args = convertToCommandLineArgs();

  LOG_INFO << "ClangASTManager configuration loaded" << std::endl;
  return true;
}

const std::string &ClangASTManager::getSourcePath() const { return sourcePath; }

const std::vector<std::string> &ClangASTManager::getCommandLineArgs() const {
  return args;
}

bool ClangASTManager::processAST(
    const std::string &source_path,
    std::function<void(clang::ASTContext &)> callback) {
  // 使用Clang的Tooling功能创建编译数据库
  int argc = args.size();
  std::vector<const char *> argv;

  // 转换参数为C风格字符串数组
  for (const auto &arg : args)
    argv.push_back(arg.c_str());

  // 创建编译数据库
  std::string errorMsg;
  std::unique_ptr<clang::tooling::FixedCompilationDatabase> compdbPtr =
      clang::tooling::FixedCompilationDatabase::loadFromCommandLine(
          argc, argv.data(), errorMsg);

  if (!errorMsg.empty())
    LOG_DEBUG << "Compilation database message: " << errorMsg << std::endl;

  if (!compdbPtr) {
    LOG_ERROR << "Failed to create compilation database: " << errorMsg
              << std::endl;
    return false;
  }
  clang::tooling::ClangTool tool(*compdbPtr, {source_path});

  // 运行工具并处理AST
  int result = tool.run(new CustomFrontendActionFactory(callback));

  if (result != 0) {
    LOG_ERROR << "Failed to process AST for: " << source_path << std::endl;
    return false;
  }

  return true;
}

std::vector<std::string> ClangASTManager::convertToCommandLineArgs() const {
  std::vector<std::string> args;

  // clang++ by default
  args.push_back("clang++");

  // FixedCompilationDatabase::loadFromCommandLine所需的
  args.push_back("--");

  // inlude PATH
  for (const auto &path : includePaths)
    args.push_back("-I" + path);

  // Macro def
  for (const auto &def : defines)
    args.push_back("-D" + def);

  // cpp std
  if (!cxxStandard.empty())
    args.push_back("-std=" + cxxStandard);

  // Other compilation flags
  args.insert(args.end(), flags.begin(), flags.end());

  // 显示已转换的参数
  LOG_DEBUG << "Command line arguments:" << std::endl;
  for (const auto &arg : args)
    LOG_DEBUG << "Arg: " << arg << std::endl;

  return args;
}