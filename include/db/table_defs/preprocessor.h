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

inline auto macroinvocations() {
  return make_table(
      "macroinvocations",
      make_column("id", &DbModel::MacroInvocation::id, primary_key()),
      make_column("macro_id", &DbModel::MacroInvocation::macro_id),
      make_column("location", &DbModel::MacroInvocation::location),
      make_column("kind", &DbModel::MacroInvocation::kind));
}

inline auto macroparent() {
  return make_table(
      "macroparent",
      make_column("id", &DbModel::MacroParent::id, primary_key()),
      make_column("parent_id", &DbModel::MacroParent::parent_id));
}

inline auto macrolocationbind() {
  return make_table(
      "macrolocationbind",
      make_column("id", &DbModel::MacroLocationBind::id),
      make_column("location", &DbModel::MacroLocationBind::location));
}

inline auto macro_argument_unexpanded() {
  return make_table(
      "macro_argument_unexpanded",
      make_column("invocation", &DbModel::MacroArgumentUnexpanded::invocation),
      make_column("argument_index",
                  &DbModel::MacroArgumentUnexpanded::argument_index),
      make_column("text", &DbModel::MacroArgumentUnexpanded::text),
      primary_key(&DbModel::MacroArgumentUnexpanded::invocation,
                  &DbModel::MacroArgumentUnexpanded::argument_index));
}

inline auto macro_argument_expanded() {
  return make_table(
      "macro_argument_expanded",
      make_column("invocation", &DbModel::MacroArgumentExpanded::invocation),
      make_column("argument_index",
                  &DbModel::MacroArgumentExpanded::argument_index),
      make_column("text", &DbModel::MacroArgumentExpanded::text),
      primary_key(&DbModel::MacroArgumentExpanded::invocation,
                  &DbModel::MacroArgumentExpanded::argument_index));
}

// clang-format on

} // namespace PreprocessorTableFn

#endif // _TABLE_DEFS_PREPROCESSOR_H_
