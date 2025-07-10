#ifndef _TABLE_DEFS_CONTAINER_H_
#define _TABLE_DEFS_CONTAINER_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/container.h"

using namespace sqlite_orm;

namespace ContainerTableFn {

// clang-format off
inline auto container() {
  return make_table(
      "container",
      make_column("id", &DbModel::Container::id, primary_key()),
      make_column("associated_id", &DbModel::Container::associated_id));
}

inline auto files() {
  return make_table(
      "files",
      make_column("id", &DbModel::File::id, primary_key()),
      make_column("name", &DbModel::File::name));
}

inline auto folders() {
  return make_table(
      "folders",
      make_column("id", &DbModel::Folder::id, primary_key()),
      make_column("name", &DbModel::Folder::name));
}

inline auto namespaces() {
  return make_table(
      "namespaces",
      make_column("id", &DbModel::Namespace::id, primary_key()),
      make_column("name", &DbModel::Namespace::name));
}

inline auto namespace_inline() {
  return make_table(
      "namespace_inline",
      make_column("id", &DbModel::NamespaceInline::id, primary_key()));
}

inline auto namespacembrs() {
  return make_table(
      "namespacembrs",
      make_column("parentid", &DbModel::NamespaceMember::parentid),
      make_column("memberid", &DbModel::NamespaceMember::memberid),
      primary_key(&DbModel::NamespaceMember::parentid, &DbModel::NamespaceMember::memberid));
}

// clang-format on

} // namespace ContainerTableFn

#endif // _TABLE_DEFS_CONTAINER_H_