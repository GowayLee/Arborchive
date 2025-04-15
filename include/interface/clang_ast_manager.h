#ifndef _CLANG_AST_MANAGER_H_
#define _CLANG_AST_MANAGER_H_

#include "model/config/configuration.h"
#include <clang/Frontend/CompilerInstance.h>
#include <memory>
#include <string>
#include <vector>

class ClangASTManager {
public:
  ClangASTManager(const ClangASTManager &) = delete;
  ClangASTManager &operator=(const ClangASTManager &) = delete;

  static ClangASTManager &getInstance() {
    static ClangASTManager instance;
    return instance;
  }

  bool loadConfig(const Configuration &config);

  // 使用Clang Tooling创建并处理AST
  bool processAST(const std::string &source_path,
                  std::function<void(clang::ASTContext &)> callback);

  const std::string &getSourcePath() const;
  const std::vector<std::string> &getCommandLineArgs() const;

private:
  ClangASTManager();
  ~ClangASTManager() = default;

  std::string sourcePath;
  std::vector<std::string> includePaths;
  std::vector<std::string> defines;
  std::string cxxStandard;
  std::vector<std::string> flags;
  std::vector<std::string> args;

  // 将配置转换为命令行参数
  std::vector<std::string> convertToCommandLineArgs() const;
};

#endif // _CLANG_AST_MANAGER_H_