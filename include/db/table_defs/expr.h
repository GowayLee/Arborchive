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

inline auto funbind() {
  return make_table(
      "funbind",
      make_column("expr", &DbModel::FunBind::expr, primary_key()),
      make_column("fun", &DbModel::FunBind::fun));
}

inline auto iscall() {
  return make_table(
      "iscall",
      make_column("caller", &DbModel::IsCall::caller, primary_key()),
      make_column("kind", &DbModel::IsCall::kind));
}

inline auto varbind() {
  return make_table(
      "varbind",
      make_column("expr", &DbModel::VarBind::expr, primary_key()),
      make_column("var", &DbModel::VarBind::var));
}

// clang-format on

} // namespace ExprTableFn

#endif // _TABLE_DEFS_EXPR_H_
