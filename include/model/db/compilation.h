#ifndef _MODEL_COMPILATION_H_
#define _MODEL_COMPILATION_H_

#include <string>

enum class CompTimeKind {
  FrontendCpu = 1,
  FrontendElapsed = 2,
  ExtractorCpu = 3,
  ExtractorElapsed = 4
};

namespace DbModel {

struct Compilation {
  int id;
  std::string cwd;
};

struct CompilationArg {
  int id;
  int num;
  std::string arg;
};

struct CompilationBuildMode {
  int id;
  int mode;
};

struct CompilationCompilingFile {
  int id;
  int num;
  int file;
};

struct CompilationTime {
  int id;
  int num;
  int kind;
  double seconds;
};

struct CompilationFinished {
  int id;
  double cpu_seconds;
  double elapsed_seconds;
};

struct ExtractorVersion {
  std::string codeql_version;
  std::string frontend_version;
};

} // namespace DbModel

#endif // _MODEL_COMPILATION_H_
