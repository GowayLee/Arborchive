#ifndef _TABLE_DEFS_COMMENT_H_
#define _TABLE_DEFS_COMMENT_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/comment.h"

using namespace sqlite_orm;

namespace CommentTableFn {

// clang-format off
inline auto comments() {
  return make_table(
      "comments",
      make_column("id", &DbModel::Comment::id, primary_key()),
      make_column("contents", &DbModel::Comment::contents),
      make_column("location", &DbModel::Comment::location));
}

inline auto commentbinding() {
  return make_table(
      "commentbinding",
      make_column("id", &DbModel::CommentBinding::id, primary_key()),
      make_column("element", &DbModel::CommentBinding::element));
}
// clang-format on

} // namespace CommentTableFn

#endif // _TABLE_DEFS_COMMENT_H_
