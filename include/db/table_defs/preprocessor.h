#ifndef _TABLE_DEFS_PREPROCESSOR_H_
#define _TABLE_DEFS_PREPROCESSOR_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/preprocessor.h"

using namespace sqlite_orm;

namespace PreprocessorTableFn {

// clang-format off

inline auto preprocdirects() {
  return make_table(
      "preprocdirects",
      make_column("id", &DbModel::Preprocdirect::id, primary_key()),
      make_column("kind", &DbModel::Preprocdirect::kind),
      make_column("location", &DbModel::Preprocdirect::location));
}

inline auto preprocpair() {
  return make_table(
      "preprocpair",
      make_column("begin", &DbModel::Preprocpair::begin),
      make_column("elseelifend", &DbModel::Preprocpair::elseelifend),
      primary_key(&DbModel::Preprocpair::begin, &DbModel::Preprocpair::elseelifend));
}

inline auto preproctrue() {
  return make_table(
      "preproctrue",
      make_column("branch", &DbModel::Preproctrue::branch, primary_key()));
}

inline auto preprocfalse() {
  return make_table(
      "preprocfalse",
      make_column("branch", &DbModel::Preprocfalse::branch, primary_key()));
}

inline auto preproctext() {
  return make_table(
      "preproctext",
      make_column("id", &DbModel::Preproctext::id, primary_key()),
      make_column("head", &DbModel::Preproctext::head),
      make_column("body", &DbModel::Preproctext::body));
}

inline auto includes() {
  return make_table(
      "includes",
      make_column("id", &DbModel::Includes::id, primary_key()),
      make_column("included", &DbModel::Includes::included));
}

// clang-format on

} // namespace PreprocessorTableFn

#endif // _TABLE_DEFS_PREPROCESSOR_H_
