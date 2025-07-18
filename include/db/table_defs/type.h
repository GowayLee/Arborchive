#ifndef _TABLE_DEFS_TYPE_H_
#define _TABLE_DEFS_TYPE_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/type.h"

using namespace sqlite_orm;

namespace TypeTableFn {

// clang-format off
// types() function deprecated - intermediary table no longer needed
inline auto types() {
  return make_table(
      "types",
      make_column("id", &DbModel::Type::id, primary_key()),
      make_column("associate_id", &DbModel::Type::associate_id),
      make_column("type", &DbModel::Type::type));
}

inline auto type_decls() {
  return make_table(
      "type_decls",
      make_column("id", &DbModel::TypeDecl::id, primary_key()),
      make_column("type_id", &DbModel::TypeDecl::type_id),
      make_column("location", &DbModel::TypeDecl::location));
}

inline auto type_def() {
  return make_table(
      "type_def",
      make_column("id", &DbModel::TypeDef::id, primary_key()));
}

inline auto type_decl_top() {
  return make_table(
      "type_decl_top",
      make_column("type_decl", &DbModel::TypeDeclTop::type_decl, primary_key()));
}

inline auto builtintypes() {
  return make_table(
      "builtintypes",
      make_column("id", &DbModel::BuiltinType_::id, primary_key()),
      make_column("name", &DbModel::BuiltinType_::name),
      make_column("kind", &DbModel::BuiltinType_::kind),
      make_column("size", &DbModel::BuiltinType_::size),
      make_column("sign", &DbModel::BuiltinType_::sign),
      make_column("alignment", &DbModel::BuiltinType_::alignment));
}

inline auto derivedtypes() {
  return make_table(
      "derivedtypes",
      make_column("id", &DbModel::DerivedType::id, primary_key()),
      make_column("name", &DbModel::DerivedType::name),
      make_column("kind", &DbModel::DerivedType::kind),
      make_column("type_id", &DbModel::DerivedType::type_id));
}

inline auto usertypes() {
  return make_table(
      "usertypes",
      make_column("id", &DbModel::UserType::id, primary_key()),
      make_column("name", &DbModel::UserType::name),
      make_column("kind", &DbModel::UserType::kind));
}

inline auto routinetypes() {
  return make_table(
      "routinetypes",
      make_column("id", &DbModel::RoutineType::id, primary_key()),
      make_column("return_type", &DbModel::RoutineType::return_type));
}

inline auto routinetypeargs() {
  return make_table(
      "routinetypeargs",
      make_column("routine", &DbModel::RoutineTypeArg::routine),
      make_column("index", &DbModel::RoutineTypeArg::index),
      make_column("type_id", &DbModel::RoutineTypeArg::type_id));
}

inline auto ptrtomembers() {
  return make_table(
      "ptrtomembers",
      make_column("id", &DbModel::PtrToMember::id, primary_key()),
      make_column("type_id", &DbModel::PtrToMember::type_id),
      make_column("class_id", &DbModel::PtrToMember::class_id));
}

inline auto decltypes() {
  return make_table(
      "decltypes",
      make_column("id", &DbModel::DeclType::id, primary_key()),
      make_column("expr", &DbModel::DeclType::expr),
      make_column("base_type", &DbModel::DeclType::base_type),
      make_column("parentheses_would_change_meaning", &DbModel::DeclType::parentheses_would_change_meaning));
}

inline auto is_pod_class() {
  return make_table(
      "is_pod_class",
      make_column("id", &DbModel::IsPodClass::id, primary_key()));
}

inline auto is_standard_layout_class() {
  return make_table(
      "is_standard_layout_class",
      make_column("id", &DbModel::IsStandartLayoutClass::id, primary_key()));
}

inline auto is_complete() {
  return make_table(
      "is_complete",
      make_column("id", &DbModel::IsComplete::id, primary_key()));
}
// clang-format on

} // namespace TypeTableFn

#endif // _TABLE_DEFS_TYPE_H_
