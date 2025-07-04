#ifndef _TABLE_DEFS_OPERATOR_H_
#define _TABLE_DEFS_OPERATOR_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/operator.h"

using namespace sqlite_orm;

namespace OpExprTableFn {

// clang-format off
inline auto op_expr() {
  return make_table(
      "op_expr",
      make_column("id", &DbModel::OpExpr::id, primary_key()),
      make_column("associate_id", &DbModel::OpExpr::associate_id),
      make_column("type", &DbModel::OpExpr::type));
}

inline auto un_op_expr() {
  return make_table(
      "un_op_expr",
      make_column("id", &DbModel::UnOpExpr::id, primary_key()),
      make_column("associate_id", &DbModel::UnOpExpr::associate_id),
      make_column("type", &DbModel::UnOpExpr::type));
}

inline auto bin_op_expr() {
  return make_table(
      "bin_op_expr",
      make_column("id", &DbModel::BinOpExpr::id, primary_key()),
      make_column("associate_id", &DbModel::BinOpExpr::associate_id),
      make_column("type", &DbModel::BinOpExpr::type));
}

// clang-format on

} // namespace OpExprTableFn

#endif // _TABLE_DEFS_OPERATOR_H_
