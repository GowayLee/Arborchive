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

// clang-format on

} // namespace DeclTableFn

#endif // _TABLE_DEFS_DECLARATION_H_
