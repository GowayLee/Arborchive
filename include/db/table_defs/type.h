#ifndef _TABLE_DEFS_TYPE_H_
#define _TABLE_DEFS_TYPE_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/type.h"

using namespace sqlite_orm;

namespace TypeTableFn {

// clang-format off
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

// clang-format on

} // namespace TypeTableFn

#endif // _TABLE_DEFS_TYPE_H_
