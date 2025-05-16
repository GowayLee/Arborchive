#ifndef _TABLE_DEFS_VARIABLE_H_
#define _TABLE_DEFS_VARIABLE_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/variable.h"

using namespace sqlite_orm;

namespace VarTableFn {

// clang-format off
inline auto variable() {
  return make_table(
      "variable",
      make_column("id", &DbModel::Variable::id, primary_key()),
      make_column("associate_id", &DbModel::Variable::associate_id),
      make_column("type", &DbModel::Variable::type));
}

inline auto localscopevariable() {
  return make_table(
      "localscopevariable",
      make_column("id", &DbModel::LocalScopeVar::id, primary_key()),
      make_column("associate_id", &DbModel::LocalScopeVar::associate_id),
      make_column("type", &DbModel::LocalScopeVar::type));
}

inline auto localvariables() {
  return make_table(
      "localvariables",
      make_column("id", &DbModel::LocalVar::id, primary_key()),
      make_column("type_id", &DbModel::LocalVar::type_id),
      make_column("name", &DbModel::LocalVar::name));
}

inline auto params() {
  return make_table(
      "params",
      make_column("id", &DbModel::Parameter::id, primary_key()),
      make_column("function", &DbModel::Parameter::function),
      make_column("index", &DbModel::Parameter::index),
      make_column("type_id", &DbModel::Parameter::type_id));
}

inline auto globalvariables() {
  return make_table(
      "globalvariables",
      make_column("id", &DbModel::GlobalVar::id, primary_key()),
      make_column("type_id", &DbModel::GlobalVar::type_id),
      make_column("name", &DbModel::GlobalVar::name));
}

inline auto membervariables() {
  return make_table(
      "membervariables",
      make_column("id", &DbModel::MemberVar::id, primary_key()),
      make_column("type_id", &DbModel::MemberVar::type_id),
      make_column("name", &DbModel::MemberVar::name));
}

inline auto var_decls() {
  return make_table(
      "var_decls",
      make_column("id", &DbModel::VarDecl::id, primary_key()),
      make_column("variable", &DbModel::VarDecl::variable),
      make_column("type_id", &DbModel::VarDecl::type_id),
      make_column("name", &DbModel::VarDecl::name),
      make_column("location", &DbModel::VarDecl::location));
}

// clang-format on

} // namespace VarTableFn

#endif // _TABLE_DEFS_VARIABLE_H_
