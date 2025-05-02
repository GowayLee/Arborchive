#ifndef _TABLE_DEFS_STMT_H_
#define _TABLE_DEFS_STMT_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/stmt.h"

using namespace sqlite_orm;

namespace StmtTableFn {

// clang-format off
inline auto stmts() {
  return make_table(
      "stmts",
      make_column("id", &DbModel::Stmt::id, primary_key()),
      make_column("kind", &DbModel::Stmt::kind),
      make_column("location", &DbModel::Stmt::location));
}

// clang-format on

} // namespace StmtTableFn

#endif // _TABLE_DEFS_STMT_H_
