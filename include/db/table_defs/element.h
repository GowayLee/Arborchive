#ifndef _TABLE_DEFS_ELEMENT_H_
#define _TABLE_DEFS_ELEMENT_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/element.h"

using namespace sqlite_orm;

namespace ElementTableFn {

// clang-format off
inline auto parameterized_element() {
  return make_table(
      "parameterized_element",
      make_column("id", &DbModel::ParameterizedElement::id, primary_key()),
      make_column("associate_id", &DbModel::ParameterizedElement::associate_id),
      make_column("type", &DbModel::ParameterizedElement::type));
}

// clang-format on

} // namespace ElementTableFn

#endif // _TABLE_DEFS_ELEMENT_H_
