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

inline auto var_def() {
  return make_table(
      "var_def",
      make_column("id", &DbModel::VarDef::id));
}

inline auto var_specialized() {
  return make_table(
      "var_specialized",
      make_column("id", &DbModel::VarSpecialized::id));
}

inline auto var_decl_specifiers() {
  return make_table(
      "var_decl_specifiers",
      make_column("id", &DbModel::VarDeclSpec::id),
      make_column("name", &DbModel::VarDeclSpec::name));
}

inline auto is_structured_binding() {
  return make_table(
      "is_structured_binding",
      make_column("id", &DbModel::IsStructuredBinding::id));
}

inline auto var_requires() {
  return make_table(
      "var_requires",
      make_column("id", &DbModel::VarRequire::id),
      make_column("constraint", &DbModel::VarRequire::constraint));
}

// clang-format on

} // namespace VarTableFn

#endif // _TABLE_DEFS_VARIABLE_H_
