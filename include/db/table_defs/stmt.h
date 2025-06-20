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

inline auto if_initalization() {
  return make_table(
      "if_initalization",
      make_column("if_stmt", &DbModel::IfInit::if_stmt, primary_key()),
      make_column("init_id", &DbModel::IfInit::init_id));
}

inline auto if_then() {
  return make_table(
      "if_then",
      make_column("if_stmt", &DbModel::IfThen::if_stmt, primary_key()),
      make_column("then_id", &DbModel::IfThen::then_id));
}

inline auto if_else() {
  return make_table(
      "if_else",
      make_column("if_stmt", &DbModel::IfElse::if_stmt, primary_key()),
      make_column("then_id", &DbModel::IfElse::else_id));
}

// clang-format on

} // namespace StmtTableFn

#endif // _TABLE_DEFS_STMT_H_
