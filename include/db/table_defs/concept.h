#ifndef _TABLE_DEFS_CONCEPT_H_
#define _TABLE_DEFS_CONCEPT_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/concept.h"

using namespace sqlite_orm;

namespace ConceptTableFn {

// clang-format off
inline auto concept_templates() {
  return make_table(
      "concept_templates",
      make_column("concept_id", &DbModel::ConceptTemplate::id, primary_key()),
      make_column("name", &DbModel::ConceptTemplate::name),
      make_column("location", &DbModel::ConceptTemplate::location));
}

inline auto concept_instantiation() {
  return make_table(
      "concept_instantiation",
      make_column("to", &DbModel::ConceptInstantiation::to, primary_key()),
      make_column("from", &DbModel::ConceptInstantiation::from));
}

inline auto concept_template_argument() {
  return make_table(
      "concept_template_argument",
      make_column("concept_id", &DbModel::ConceptTemplateArgument::concept_id),
      make_column("index", &DbModel::ConceptTemplateArgument::index),
      make_column("arg_type", &DbModel::ConceptTemplateArgument::arg_type));
}
// clang-format on

} // namespace ConceptTableFn

#endif // _TABLE_DEFS_CONCEPT_H_
