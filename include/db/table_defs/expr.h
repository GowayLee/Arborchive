#ifndef _TABLE_DEFS_EXPR_H_
#define _TABLE_DEFS_EXPR_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/expr.h"

using namespace sqlite_orm;

namespace ExprTableFn {

// clang-format off
inline auto exprs() {
  return make_table(
      "exprs",
      make_column("id", &DbModel::Expr::id, primary_key()),
      make_column("kind", &DbModel::Expr::kind),
      make_column("location", &DbModel::Expr::location));
}

// clang-format on

} // namespace ExprTableFn

#endif // _TABLE_DEFS_EXPR_H_
