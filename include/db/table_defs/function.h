#ifndef _TABLE_DEFS_FUNCTION_H_
#define _TABLE_DEFS_FUNCTION_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/function.h"

using namespace sqlite_orm;

namespace FuncTableFn {

// clang-format off
inline auto functions() {
  return make_table(
      "functions",
      make_column("id", &DbModel::Function::id, primary_key()),
      make_column("name", &DbModel::Function::name),
      make_column("kind", &DbModel::Function::kind));
}

// clang-format on

} // namespace FuncTableFn

#endif // _TABLE_DEFS_FUNCTION_H_
