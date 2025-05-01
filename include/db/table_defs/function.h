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

// clang-format on

} // namespace FuncTableFn

#endif // _TABLE_DEFS_FUNCTION_H_
