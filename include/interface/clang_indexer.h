#ifndef _LIBCLANG_INITIALIZER_H_
#define _LIBCLANG_INITIALIZER_H_

#include "model/config/configuration.h"
#include <clang-c/Index.h>
#include <string>
#include <vector>

class ClangIndexer {
public:
  ClangIndexer(const ClangIndexer &) = delete;
  ClangIndexer &operator=(const ClangIndexer &) = delete;

  static ClangIndexer &getInstance() {
    static ClangIndexer instance;
    return instance;
  }

  bool loadConfig(const Configuration &config);
  bool init();

  // 获取初始化后的CXIndex
  CXIndex getIndex() const;
  const std::string &getSourcePath() const;
  const char *const *getCommandLineArgs() const;
  size_t getCommandLineArgsCount() const;

  // 新增: 创建翻译单元
  CXTranslationUnit createTranslationUnit(const std::string &source_path);

private:
  ClangIndexer();
  ~ClangIndexer();

  std::string sourcePath;
  std::vector<std::string> includePaths;
  std::vector<std::string> defines;
  std::string cxxStandard;
  std::vector<std::string> flags;

  std::vector<std::string> args;
  std::vector<const char *> c_args;

  CXIndex index;

  std::vector<std::string> convertToCommandLineArgs() const;
};

#endif // _LIBCLANG_INITIALIZER_H_
