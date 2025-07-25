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

// inline auto stmt_for_or_range_based_for() {
//   return make_table(
//       "stmt_for_or_range_based_for",
//       make_column("id", &DbModel::StmtForOrRangeBased::id, primary_key()),
//       make_column("associated_id", &DbModel::StmtForOrRangeBased::associated_id),
//       make_column("type", &DbModel::StmtForOrRangeBased::type));
// }

inline auto for_initialization() {
  return make_table(
      "for_initialization",
      make_column("for_stmt", &DbModel::ForInit::for_stmt, primary_key()),
      make_column("init_id", &DbModel::ForInit::init_id));
}

inline auto for_condition() {
  return make_table(
      "for_condition",
      make_column("for_stmt", &DbModel::ForCond::for_stmt, primary_key()),
      make_column("condition_id", &DbModel::ForCond::condition_id));
}

inline auto for_update() {
  return make_table(
      "for_update",
      make_column("for_stmt", &DbModel::ForUpdate::for_stmt, primary_key()),
      make_column("update_id", &DbModel::ForUpdate::update_id));
}

inline auto for_body() {
  return make_table(
      "for_body",
      make_column("for_stmt", &DbModel::ForBody::for_stmt, primary_key()),
      make_column("body_id", &DbModel::ForBody::body_id));
}

inline auto while_body() {
  return make_table(
      "while_body",
      make_column("while_stmt", &DbModel::WhileBody::while_stmt, primary_key()),
      make_column("body_id", &DbModel::WhileBody::body_id));
}

inline auto do_body() {
  return make_table(
      "do_body",
      make_column("do_stmt", &DbModel::DoBody::do_stmt, primary_key()),
      make_column("body_id", &DbModel::DoBody::body_id));
}

inline auto switch_initialization() {
  return make_table(
      "switch_initialization",
      make_column("switch_stmt", &DbModel::SwitchInit::switch_stmt, primary_key()),
      make_column("init_id", &DbModel::SwitchInit::init_id));
}

inline auto switch_case() {
  return make_table(
      "switch_case",
      make_column("switch_stmt", &DbModel::SwitchCase::switch_stmt),
      make_column("index", &DbModel::SwitchCase::index),
      make_column("case_id", &DbModel::SwitchCase::case_id, primary_key()));
}

inline auto switch_body() {
  return make_table(
      "switch_body",
      make_column("switch_stmt", &DbModel::SwitchBody::switch_stmt, primary_key()),
      make_column("body_id", &DbModel::SwitchBody::body_id));
}

// clang-format on

} // namespace StmtTableFn

#endif // _TABLE_DEFS_STMT_H_
