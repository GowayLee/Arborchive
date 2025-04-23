#ifndef _MODEL_COMPILATION_H_
#define _MODEL_COMPILATION_H_

#include <string>

enum class CompilationTimeKind {
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

} // namespace DbModel

#endif // _MODEL_COMPILATION_H_