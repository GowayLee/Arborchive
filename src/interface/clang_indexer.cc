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
  sourcePath = config.general.source_path;
  includePaths = config.compilation.include_paths;
  defines = config.compilation.defines;
  cxxStandard = config.compilation.cxx_standard;
  flags = config.compilation.flags;
  LOG_INFO << "ClangIndexer configuration loaded" << std::endl;
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
  for (const auto &arg : args)
    c_args.push_back(arg.c_str());

  return true;
}

CXIndex ClangIndexer::getIndex() const { return index; }

CXTranslationUnit
ClangIndexer::createTranslationUnit(const std::string &source_path) {
  CXTranslationUnit tu = clang_parseTranslationUnit(
      index, source_path.c_str(), c_args.data(),
      static_cast<int>(c_args.size()), nullptr, 0, CXTranslationUnit_None);

  if (!tu) {
    LOG_ERROR << "Failed to create translation unit for: " << source_path
              << std::endl;
    return nullptr;
  }

  unsigned num_diags = clang_getNumDiagnostics(tu);
  if (num_diags > 0) {
    LOG_WARNING << "Translation unit has " << num_diags << " diagnostics"
                << std::endl;
    for (unsigned i = 0; i < num_diags; ++i) {
      CXDiagnostic diag = clang_getDiagnostic(tu, i);
      CXString diag_str =
          clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
      LOG_WARNING << clang_getCString(diag_str) << std::endl;
      clang_disposeString(diag_str);
      clang_disposeDiagnostic(diag);
    }
  }

  return tu;
}

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
