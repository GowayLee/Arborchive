#ifndef _TABLE_DEFS_SPECIFIERS_H_
#define _TABLE_DEFS_SPECIFIERS_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/specifiers.h"

using namespace sqlite_orm;

namespace SpecifierTableFn {

// clang-format off

inline auto specifiers() {
  return make_table(
      "specifiers",
      make_column("id", &DbModel::Specifier::id, primary_key()),
      make_column("str", &DbModel::Specifier::str, unique()));
}

inline auto typespecifiers() {
  return make_table(
      "typespecifiers",
      make_column("type_id", &DbModel::TypeSpecifiers::type_id),
      make_column("spec_id", &DbModel::TypeSpecifiers::spec_id));
}

inline auto funspecifiers() {
  return make_table(
      "funspecifiers",
      make_column("func_id", &DbModel::FunSpecifiers::func_id),
      make_column("spec_id", &DbModel::FunSpecifiers::spec_id));
}

inline auto varspecifiers() {
  return make_table(
      "varspecifiers",
      make_column("var_id", &DbModel::VarSpecifiers::var_id),
      make_column("spec_id", &DbModel::VarSpecifiers::spec_id));
}

// clang-format on

} // namespace SpecifierTableFn

#endif // _TABLE_DEFS_SPECIFIERS_H_
