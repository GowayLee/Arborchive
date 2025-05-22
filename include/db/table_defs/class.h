#ifndef _TABLE_DEFS_CLASS_H_
#define _TABLE_DEFS_CLASS_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/class.h"

using namespace sqlite_orm;

namespace ClassTableFn {

// clang-format off
inline auto memeber() {
  return make_table(
      "member",
      make_column("id", &DbModel::Member::id, primary_key()),
      make_column("associated_id", &DbModel::Member::associated_id),
      make_column("type", &DbModel::Member::type));
}

// clang-format on

} // namespace DeclTableFn

#endif // _TABLE_DEFS_CLASS_H_
