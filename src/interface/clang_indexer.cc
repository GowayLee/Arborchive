#include "interface/clang_indexer.h"
#include "util/logger/macros.h"
#include <clang-c/Index.h>
#include <string>

ClangIndexer::ClangIndexer() : index(nullptr) {}

ClangIndexer::~ClangIndexer() {
  if (index)
    clang_disposeIndex(index);
}

bool ClangIndexer::loadConfig(const Configuration &config) {
  // 根据配置文件设置相应的成员变量
  sourcePath = config.general.source_path;
  includePaths = config.compilation.include_paths;
  defines = config.compilation.defines;
  cxxStandard = config.compilation.cxx_standard;
  flags = config.compilation.flags;
  LOG_INFO << "ClangIndexer configuration loaded" << std::endl;

  // initialize
  return init();
}

const std::string &ClangIndexer::getSourcePath() const { return sourcePath; }
const char *const *ClangIndexer::getCommandLineArgs() const {
  return c_args.data();
}
size_t ClangIndexer::getCommandLineArgsCount() const { return c_args.size(); }

bool ClangIndexer::init() {
  if (index)
    clang_disposeIndex(index);

  index = clang_createIndex(0, 0);
  if (!index) {
    LOG_ERROR << "Failed to create CXIndex" << std::endl;
    return false;
  }

  args = convertToCommandLineArgs();

  // Convert args to C-style strings,
  for (const auto &arg : args)
    c_args.push_back(arg.c_str());

  // CXTranslationUnit tu = clang_parseTranslationUnit(
  //     index, sourcePath.c_str(), c_args.data(), c_args.size(), nullptr, 0,
  //     CXTranslationUnit_None);

  // Display converted args in log
  for (const auto &arg : args)
    LOG_DEBUG << "Arg: " << arg << std::endl;

  return true;
}

CXIndex ClangIndexer::getIndex() const { return index; }

std::vector<std::string> ClangIndexer::convertToCommandLineArgs() const {
  std::vector<std::string> args;

  for (const auto &path : includePaths)
    args.push_back("-I" + path);

  for (const auto &def : defines)
    args.push_back("-D" + def);

  if (!cxxStandard.empty())
    args.push_back("-std=" + cxxStandard);

  args.insert(args.end(), flags.begin(), flags.end());

  return args;
}
