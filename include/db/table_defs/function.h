#ifndef _TABLE_DEFS_FUNCTION_H_
#define _TABLE_DEFS_FUNCTION_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/function.h"

using namespace sqlite_orm;

namespace FuncTableFn {

// clang-format off
inline auto functions() {
  return make_table(
      "functions",
      make_column("id", &DbModel::Function::id, primary_key()),
      make_column("name", &DbModel::Function::name),
      make_column("kind", &DbModel::Function::kind));
}

inline auto fun_decls() {
  return make_table(
      "fun_decls",
      make_column("id", &DbModel::FunDecl::id, primary_key()),
      make_column("function", &DbModel::FunDecl::function),
      make_column("type_id", &DbModel::FunDecl::type_id),
      make_column("name", &DbModel::FunDecl::name),
      make_column("location", &DbModel::FunDecl::location));
}

inline auto fun_def() {
  return make_table(
      "fun_def",
      make_column("id", &DbModel::FunDef::id, primary_key()));
}

inline auto function_return_type() {
  return make_table(
      "function_return_type",
      make_column("id", &DbModel::FuncRetType::id),
      make_column("return_type", &DbModel::FuncRetType::return_type));
}

inline auto pure_functions() {
  return make_table(
      "purefunctions",
      make_column("id", &DbModel::PureFuncs::id));
}

inline auto function_deleted() {
  return make_table(
      "function_deleted",
      make_column("id", &DbModel::FuncDeleted::id));
}

inline auto function_defaulted() {
  return make_table(
      "function_defaulted",
      make_column("id", &DbModel::FuncDefaulted::id));
}

inline auto function_prototyped() {
  return make_table(
      "function_prototyped",
      make_column("id", &DbModel::FuncPrototyped::id));
}

inline auto fun_specialized() {
  return make_table(
      "fun_specialized",
      make_column("id", &DbModel::FunSpecialized::id));
}

inline auto fun_implicit() {
  return make_table(
      "fun_implicit",
      make_column("id", &DbModel::FunImplicit::id));
}

inline auto function_entry_point() {
  return make_table(
      "function_entry_point",
      make_column("id", &DbModel::FuncEntryPt::id),
      make_column("entry_point", &DbModel::FuncEntryPt::entry_point));
}

inline auto deduction_guide_for_class() {
  return make_table(
      "deduction_guide_for_class",
      make_column("id", &DbModel::DeductionGuideForClass::id),
      make_column("class_template", &DbModel::DeductionGuideForClass::class_template));
}

// clang-format on

} // namespace FuncTableFn

#endif // _TABLE_DEFS_FUNCTION_H_
