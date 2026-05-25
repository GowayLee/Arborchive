#ifndef _TABLE_DEFS_DECLARATION_H_
#define _TABLE_DEFS_DECLARATION_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/declaration.h"

using namespace sqlite_orm;

namespace DeclTableFn {

// clang-format off
inline auto declarations() {
  return make_table(
      "declarations",
      make_column("id", &DbModel::Declaration::id, primary_key()),
      make_column("associated_id", &DbModel::Declaration::associated_id),
      make_column("type", &DbModel::Declaration::type));
}

inline auto frienddecls() {
  return make_table(
      "frienddecls",
      make_column("id", &DbModel::FriendDecl::id, primary_key()),
      make_column("type_id", &DbModel::FriendDecl::type_id),
      make_column("decl_id", &DbModel::FriendDecl::decl_id),
      make_column("location", &DbModel::FriendDecl::location));
}

inline auto namespace_decls() {
  return make_table(
      "namespace_decls",
      make_column("id", &DbModel::NamespaceDecl::id, primary_key()),
      make_column("namespace_id", &DbModel::NamespaceDecl::namespace_id),
      make_column("location", &DbModel::NamespaceDecl::location),
      make_column("bodylocation", &DbModel::NamespaceDecl::bodylocation));
}

inline auto usings() {
  return make_table(
      "usings",
      make_column("id", &DbModel::Using::id, primary_key()),
      make_column("element_id", &DbModel::Using::element_id),
      make_column("location", &DbModel::Using::location),
      make_column("kind", &DbModel::Using::kind));
}

inline auto using_container() {
  return make_table(
      "using_container",
      make_column("parent", &DbModel::UsingContainer::parent),
      make_column("child", &DbModel::UsingContainer::child));
}

// clang-format on

} // namespace DeclTableFn

#endif // _TABLE_DEFS_DECLARATION_H_
