#ifndef _TABLE_DEFS_LOCATION_H_
#define _TABLE_DEFS_LOCATION_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/location.h"

using namespace sqlite_orm;

namespace LocTableFn {

// clang-format off
inline auto locations() {
  return make_table(
      "locations",
      make_column("id", &DbModel::Location::id, primary_key()),
      make_column("associated_ed", &DbModel::Location::associated_id));
}

inline auto locations_default() {
  return make_table(
      "locations_default",
      make_column("id", &DbModel::LocationDefault::id, primary_key()),
      make_column("container", &DbModel::LocationDefault::container),
      make_column("start_line", &DbModel::LocationDefault::start_line),
      make_column("start_column", &DbModel::LocationDefault::start_column),
      make_column("end_line", &DbModel::LocationDefault::end_line),
      make_column("end_column", &DbModel::LocationDefault::end_column));
}

inline auto locations_stmt() {
  return make_table(
      "locations_stmt",
      make_column("id", &DbModel::LocationStmt::id, primary_key()),
      make_column("container", &DbModel::LocationStmt::container),
      make_column("start_line", &DbModel::LocationStmt::start_line),
      make_column("start_column", &DbModel::LocationStmt::start_column),
      make_column("end_line", &DbModel::LocationStmt::end_line),
      make_column("end_column", &DbModel::LocationStmt::end_column));
}
inline auto locations_expr() {
  return make_table(
      "locations_expr",
      make_column("id", &DbModel::LocationExpr::id, primary_key()),
      make_column("container", &DbModel::LocationExpr::container),
      make_column("start_line", &DbModel::LocationExpr::start_line),
      make_column("start_column", &DbModel::LocationExpr::start_column),
      make_column("end_line", &DbModel::LocationExpr::end_line),
      make_column("end_column", &DbModel::LocationExpr::end_column));
}

// clang-format on

} // namespace LocTableFn

#endif // _TABLE_DEFS_LOCATION_H_