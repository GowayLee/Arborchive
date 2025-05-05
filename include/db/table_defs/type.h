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
