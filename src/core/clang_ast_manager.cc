#include "core/clang_ast_manager.h"
#include "core/processor/preprocessor_processor.h"
#include "util/logger/macros.h"
#include <clang/Driver/Driver.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <filesystem>
#include <memory>

#ifndef ARBORCHIVE_CLANG_EXECUTABLE
#error "ARBORCHIVE_CLANG_EXECUTABLE must name the LLVM 19 clang++ driver"
#endif

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
    // Install PPCallbacks for preprocessor directive tracking
    auto &PP = CI.getPreprocessor();
    PP.addPPCallbacks(std::make_unique<PreprocessorProcessor>(
        &CI.getASTContext(),
        CI.getASTContext().getPrintingPolicy(),
        &PP));

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
  if (args.size() < 3 || args.front().empty() || args.back() != source_path) {
    LOG_ERROR << "Invalid compiler command line for: " << source_path
              << std::endl;
    return false;
  }

  // FixedCompilationDatabase supplies argv[0] and the positional source path.
  // Feed it the middle of the same complete sequence exposed to the recorder.
  std::vector<std::string> compiler_options(args.begin() + 1, args.end() - 1);
  clang::tooling::FixedCompilationDatabase compilation_database(
      std::filesystem::current_path().string(), compiler_options);
  clang::tooling::ClangTool tool(compilation_database, {source_path});

  // ClangTool installs ClangSyntaxOnlyAdjuster by default, which would append
  // an unrecorded -fsyntax-only argument. Keep execution and compilation_args
  // on the same sequence before applying the argv[0] restoration below.
  tool.clearArgumentsAdjusters();

  // FixedCompilationDatabase hard-codes argv[0] to "clang-tool". Restore the
  // LLVM 19 driver used to build Arborchive so Clang can discover its C++
  // standard library, platform SDK, and builtin resource headers.
  const std::string clang_executable = args.front();
  tool.appendArgumentsAdjuster(
      [clang_executable](
          const clang::tooling::CommandLineArguments &arguments,
          llvm::StringRef) {
        auto adjusted = arguments;
        if (adjusted.empty())
          return adjusted;

        adjusted[0] = clang_executable;
        return adjusted;
      });

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

  const std::string clang_executable = ARBORCHIVE_CLANG_EXECUTABLE;
  args.push_back(clang_executable);
  args.push_back("-resource-dir=" +
                 clang::driver::Driver::GetResourcesPath(clang_executable));

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

  // Arborchive currently accepts exactly one C++ source per invocation.
  args.push_back(sourcePath);

  // 显示已转换的参数
  LOG_DEBUG << "Command line arguments:" << std::endl;
  for (const auto &arg : args)
    LOG_DEBUG << "Arg: " << arg << std::endl;

  return args;
}
