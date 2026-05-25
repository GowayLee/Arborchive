#ifndef _TABLE_DEFS_VARIABLE_H_
#define _TABLE_DEFS_VARIABLE_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/variable.h"

using namespace sqlite_orm;

namespace VarTableFn {

// clang-format off
// TO BE REMOVED: variable() function - intermediary table no longer needed
 inline auto variable() {
   return make_table(
       "variable",
       make_column("id", &DbModel::Variable::id, primary_key()),
       make_column("associate_id", &DbModel::Variable::associate_id),
       make_column("type", &DbModel::Variable::type));
 }

// TO BE REMOVED: localscopevariable() function - intermediary table no longer needed
// inline auto localscopevariable() {
//   return make_table(
//       "localscopevariable",
//       make_column("id", &DbModel::LocalScopeVar::id, primary_key()),
//       make_column("associate_id", &DbModel::LocalScopeVar::associate_id),
//       make_column("type", &DbModel::LocalScopeVar::type));
// }

inline auto localvariables() {
  return make_table(
      "localvariables",
      make_column("id", &DbModel::LocalVar::id, primary_key()),
      make_column("type_id", &DbModel::LocalVar::type_id),
      make_column("name", &DbModel::LocalVar::name));
}

inline auto params() {
  return make_table(
      "params",
      make_column("id", &DbModel::Parameter::id, primary_key()),
      make_column("function", &DbModel::Parameter::function),
      make_column("index", &DbModel::Parameter::index),
      make_column("type_id", &DbModel::Parameter::type_id));
}

inline auto globalvariables() {
  return make_table(
      "globalvariables",
      make_column("id", &DbModel::GlobalVar::id, primary_key()),
      make_column("type_id", &DbModel::GlobalVar::type_id),
      make_column("name", &DbModel::GlobalVar::name));
}

inline auto membervariables() {
  return make_table(
      "membervariables",
      make_column("id", &DbModel::MemberVar::id, primary_key()),
      make_column("type_id", &DbModel::MemberVar::type_id),
      make_column("name", &DbModel::MemberVar::name));
}

inline auto var_decls() {
  return make_table(
      "var_decls",
      make_column("id", &DbModel::VarDecl::id, primary_key()),
      make_column("variable", &DbModel::VarDecl::variable),
      make_column("type_id", &DbModel::VarDecl::type_id),
      make_column("name", &DbModel::VarDecl::name),
      make_column("location", &DbModel::VarDecl::location));
}

inline auto var_def() {
  return make_table(
      "var_def",
      make_column("id", &DbModel::VarDef::id));
}

inline auto var_specialized() {
  return make_table(
      "var_specialized",
      make_column("id", &DbModel::VarSpecialized::id));
}

inline auto is_variable_template() {
  return make_table(
      "is_variable_template",
      make_column("id", &DbModel::IsVariableTemplate::id, primary_key()));
}

inline auto variable_instantiation() {
  return make_table(
      "variable_instantiation",
      make_column("to", &DbModel::VariableInstantiation::to),
      make_column("from", &DbModel::VariableInstantiation::from));
}

inline auto variable_template_argument() {
  return make_table(
      "variable_template_argument",
      make_column("variable_id", &DbModel::VariableTemplateArgument::variable_id),
      make_column("index", &DbModel::VariableTemplateArgument::index),
      make_column("arg_type", &DbModel::VariableTemplateArgument::arg_type));
}

inline auto variable_template_argument_value() {
  return make_table(
      "variable_template_argument_value",
      make_column("variable_id",
                  &DbModel::VariableTemplateArgumentValue::variable_id),
      make_column("index", &DbModel::VariableTemplateArgumentValue::index),
      make_column("arg_value",
                  &DbModel::VariableTemplateArgumentValue::arg_value));
}

inline auto var_decl_specifiers() {
  return make_table(
      "var_decl_specifiers",
      make_column("id", &DbModel::VarDeclSpec::id),
      make_column("name", &DbModel::VarDeclSpec::name));
}

inline auto is_structured_binding() {
  return make_table(
      "is_structured_binding",
      make_column("id", &DbModel::IsStructuredBinding::id));
}

inline auto var_requires() {
  return make_table(
      "var_requires",
      make_column("id", &DbModel::VarRequire::id),
      make_column("constraint", &DbModel::VarRequire::constraint));
}

inline auto fieldoffsets() {
  return make_table(
      "fieldoffsets",
      make_column("id", &DbModel::FieldOffset::id, primary_key()),
      make_column("byteoffset", &DbModel::FieldOffset::byteoffset),
      make_column("bitoffset", &DbModel::FieldOffset::bitoffset));
}

inline auto bitfield() {
  return make_table(
      "bitfield",
      make_column("id", &DbModel::BitField::id, primary_key()),
      make_column("bits", &DbModel::BitField::bits),
      make_column("declared_bits", &DbModel::BitField::declared_bits));
}

inline auto arbor_field_layout_traits() {
  return make_table(
      "arbor_field_layout_traits",
      make_column("id", &DbModel::ArborFieldLayoutTrait::id, primary_key()),
      make_column("is_bitfield", &DbModel::ArborFieldLayoutTrait::is_bitfield),
      make_column("is_zero_size", &DbModel::ArborFieldLayoutTrait::is_zero_size),
      make_column("is_potentially_overlapping",
                  &DbModel::ArborFieldLayoutTrait::is_potentially_overlapping),
      make_column("has_no_unique_address",
                  &DbModel::ArborFieldLayoutTrait::has_no_unique_address),
      make_column("is_anonymous_struct_or_union",
                  &DbModel::ArborFieldLayoutTrait::is_anonymous_struct_or_union),
      make_column("parent_is_union",
                  &DbModel::ArborFieldLayoutTrait::parent_is_union));
}

inline auto arbor_indirect_field_paths() {
  return make_table(
      "arbor_indirect_field_paths",
      make_column("id", &DbModel::ArborIndirectFieldPath::id, primary_key()),
      make_column("parent", &DbModel::ArborIndirectFieldPath::parent),
      make_column("leaf", &DbModel::ArborIndirectFieldPath::leaf),
      make_column("name", &DbModel::ArborIndirectFieldPath::name),
      make_column("path", &DbModel::ArborIndirectFieldPath::path),
      make_column("field_count", &DbModel::ArborIndirectFieldPath::field_count));
}

// clang-format on

} // namespace VarTableFn

#endif // _TABLE_DEFS_VARIABLE_H_
