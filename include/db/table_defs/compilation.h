#ifndef _TABLE_DEFS_COMPILATION_H_
#define _TABLE_DEFS_COMPILATION_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/compilation.h"

using namespace sqlite_orm;

namespace CompTableFn {

// clang-format off
inline auto compilations() {
  return make_table(
      "compilations",
      make_column("id", &DbModel::Compilation::id, primary_key()),
      make_column("cwd", &DbModel::Compilation::cwd));
}

inline auto compilatio_args() {
  return make_table(
      "compilation_args",
      make_column("id", &DbModel::CompilationArg::id),
      make_column("num", &DbModel::CompilationArg::num),
      make_column("arg", &DbModel::CompilationArg::arg),
      primary_key(&DbModel::CompilationArg::id,
                  &DbModel::CompilationArg::num));
}

inline auto compilatio_build_mode() {
  return make_table(
      "compilation_build_mode",
      make_column("id", &DbModel::CompilationBuildMode::id, primary_key()),
      make_column("mode", &DbModel::CompilationBuildMode::mode));
}

inline auto compilation_compiling_files() {
  return make_table(
      "compilation_compiling_files",
      make_column("id", &DbModel::CompilationCompilingFile::id),
      make_column("num", &DbModel::CompilationCompilingFile::num),
      make_column("file", &DbModel::CompilationCompilingFile::file),
      primary_key(&DbModel::CompilationCompilingFile::id,
                  &DbModel::CompilationCompilingFile::num));
}

inline auto compilatio_time() {
  return make_table(
      "compilation_time",
      make_column("id", &DbModel::CompilationTime::id),
      make_column("num", &DbModel::CompilationTime::num),
      make_column("kind", &DbModel::CompilationTime::kind),
      make_column("seconds", &DbModel::CompilationTime::seconds),
      primary_key(&DbModel::CompilationTime::id,
                  &DbModel::CompilationTime::num,
                  &DbModel::CompilationTime::kind));
}

inline auto compilation_finished() {
  return make_table(
      "compilation_finished",
      make_column("id", &DbModel::CompilationFinished::id, primary_key()),
      make_column("cpu_seconds", &DbModel::CompilationFinished::cpu_seconds),
      make_column("elapsed_seconds", &DbModel::CompilationFinished::elapsed_seconds));
}


inline auto extractor_version() {
  return make_table(
      "extractor_version",
      make_column("codeql_version", &DbModel::ExtractorVersion::codeql_version),
      make_column("frontend_version", &DbModel::ExtractorVersion::frontend_version));
}
// clang-format on

} // namespace TableFn

#endif // _TABLE_DEFS_COMPILATION_H_
