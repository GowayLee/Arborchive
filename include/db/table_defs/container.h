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

// clang-format on

} // namespace ContainerTableFn

#endif // _TABLE_DEFS_CONTAINER_H_