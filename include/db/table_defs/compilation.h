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
      make_column("arg", &DbModel::CompilationArg::arg));
}

inline auto compilatio_build_mode() {
  return make_table(
      "compilation_build_mode",
      make_column("id", &DbModel::CompilationBuildMode::id, primary_key()),
      make_column("mode", &DbModel::CompilationBuildMode::mode));
}

inline auto compilatio_time() {
  return make_table(
      "compilation_time",
      make_column("id", &DbModel::CompilationTime::id),
      make_column("num", &DbModel::CompilationTime::num),
      make_column("kind", &DbModel::CompilationTime::kind),
      make_column("seconds", &DbModel::CompilationTime::seconds));
}

inline auto compilation_finished() {
  return make_table(
      "compilation_finished",
      make_column("id", &DbModel::CompilationFinished::id, primary_key()),
      make_column("cpu_seconds", &DbModel::CompilationFinished::cpu_seconds),
      make_column("elapsed_seconds", &DbModel::CompilationFinished::elapsed_seconds));
}
// clang-format on

} // namespace TableFn

#endif // _TABLE_DEFS_COMPILATION_H_