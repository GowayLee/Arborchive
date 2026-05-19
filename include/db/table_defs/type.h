#ifndef _TABLE_DEFS_TYPE_H_
#define _TABLE_DEFS_TYPE_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/type.h"
#include "model/db/location.h"

using namespace sqlite_orm;

namespace TypeTableFn {

// clang-format off
// types() function deprecated - intermediary table no longer needed
inline auto types() {
  return make_table(
      "types",
      make_column("id", &DbModel::Type::id, primary_key()),
      make_column("associate_id", &DbModel::Type::associate_id),
      make_column("type", &DbModel::Type::type));
}

inline auto type_decls() {
  return make_table(
      "type_decls",
      make_column("id", &DbModel::TypeDecl::id, primary_key()),
      make_column("type_id", &DbModel::TypeDecl::type_id),
      make_column("location", &DbModel::TypeDecl::location));
}

inline auto type_def() {
  return make_table(
      "type_def",
      make_column("id", &DbModel::TypeDef::id, primary_key()));
}

inline auto type_decl_top() {
  return make_table(
      "type_decl_top",
      make_column("type_decl", &DbModel::TypeDeclTop::type_decl, primary_key()));
}

inline auto builtintypes() {
  return make_table(
      "builtintypes",
      make_column("id", &DbModel::BuiltinType_::id, primary_key()),
      make_column("name", &DbModel::BuiltinType_::name),
      make_column("kind", &DbModel::BuiltinType_::kind),
      make_column("size", &DbModel::BuiltinType_::size),
      make_column("sign", &DbModel::BuiltinType_::sign),
      make_column("alignment", &DbModel::BuiltinType_::alignment));
}

inline auto derivedtypes() {
  return make_table(
      "derivedtypes",
      make_column("id", &DbModel::DerivedType::id, primary_key()),
      make_column("name", &DbModel::DerivedType::name),
      make_column("kind", &DbModel::DerivedType::kind),
      make_column("type_id", &DbModel::DerivedType::type_id));
}

inline auto usertypes() {
  return make_table(
      "usertypes",
      make_column("id", &DbModel::UserType::id, primary_key()),
      make_column("name", &DbModel::UserType::name),
      make_column("kind", &DbModel::UserType::kind));
}

inline auto routinetypes() {
  return make_table(
      "routinetypes",
      make_column("id", &DbModel::RoutineType::id, primary_key()),
      make_column("return_type", &DbModel::RoutineType::return_type));
}

inline auto routinetypeargs() {
  return make_table(
      "routinetypeargs",
      make_column("routine", &DbModel::RoutineTypeArg::routine),
      make_column("index", &DbModel::RoutineTypeArg::index),
      make_column("type_id", &DbModel::RoutineTypeArg::type_id));
}

inline auto ptrtomembers() {
  return make_table(
      "ptrtomembers",
      make_column("id", &DbModel::PtrToMember::id, primary_key()),
      make_column("type_id", &DbModel::PtrToMember::type_id),
      make_column("class_id", &DbModel::PtrToMember::class_id));
}

inline auto decltypes() {
  return make_table(
      "decltypes",
      make_column("id", &DbModel::DeclType::id, primary_key()),
      make_column("expr", &DbModel::DeclType::expr),
      make_column("base_type", &DbModel::DeclType::base_type),
      make_column("parentheses_would_change_meaning", &DbModel::DeclType::parentheses_would_change_meaning));
}

inline auto is_pod_class() {
  return make_table(
      "is_pod_class",
      make_column("id", &DbModel::IsPodClass::id, primary_key()));
}

inline auto is_standard_layout_class() {
  return make_table(
      "is_standard_layout_class",
      make_column("id", &DbModel::IsStandartLayoutClass::id, primary_key()));
}

inline auto is_complete() {
  return make_table(
      "is_complete",
      make_column("id", &DbModel::IsComplete::id, primary_key()));
}

inline auto is_class_template() {
  return make_table(
      "is_class_template",
      make_column("id", &DbModel::IsClassTemplate::id, primary_key()));
}

inline auto class_instantiation() {
  return make_table(
      "class_instantiation",
      make_column("to", &DbModel::ClassInstantiation::to),
      make_column("from", &DbModel::ClassInstantiation::from));
}

inline auto class_template_argument() {
  return make_table(
      "class_template_argument",
      make_column("type_id", &DbModel::ClassTemplateArgument::type_id),
      make_column("index", &DbModel::ClassTemplateArgument::index),
      make_column("arg_type", &DbModel::ClassTemplateArgument::arg_type));
}

inline auto class_template_argument_value() {
  return make_table(
      "class_template_argument_value",
      make_column("type_id", &DbModel::ClassTemplateArgumentValue::type_id),
      make_column("index", &DbModel::ClassTemplateArgumentValue::index),
      make_column("arg_value", &DbModel::ClassTemplateArgumentValue::arg_value));
}

inline auto template_template_instantiation() {
  return make_table(
      "template_template_instantiation",
      make_column("to", &DbModel::TemplateTemplateInstantiation::to),
      make_column("from", &DbModel::TemplateTemplateInstantiation::from));
}

inline auto template_template_argument() {
  return make_table(
      "template_template_argument",
      make_column("type_id", &DbModel::TemplateTemplateArgument::type_id),
      make_column("index", &DbModel::TemplateTemplateArgument::index),
      make_column("arg_type", &DbModel::TemplateTemplateArgument::arg_type));
}

inline auto type_template_type_constraint() {
  return make_table(
      "type_template_type_constraint",
      make_column("id", &DbModel::TypeTemplateTypeConstraint::id),
      make_column("constraint",
                  &DbModel::TypeTemplateTypeConstraint::constraint));
}

// New C language feature tables
inline auto enumconstants() {
  return make_table(
      "enumconstants",
      make_column("id", &DbModel::EnumConstant::id, primary_key()),
      make_column("parent", &DbModel::EnumConstant::parent),
      make_column("index", &DbModel::EnumConstant::index),
      make_column("type_id", &DbModel::EnumConstant::type_id),
      make_column("name", &DbModel::EnumConstant::name),
      make_column("location", &DbModel::EnumConstant::location));
}

inline auto typedefbase() {
  return make_table(
      "typedefbase",
      make_column("id", &DbModel::TypedefBase::id, primary_key()),
      make_column("type_id", &DbModel::TypedefBase::type_id));
}

inline auto arraysizes() {
  return make_table(
      "arraysizes",
      make_column("id", &DbModel::ArraySizes::id, primary_key()),
      make_column("num_elements", &DbModel::ArraySizes::num_elements),
      make_column("bytesize", &DbModel::ArraySizes::bytesize),
      make_column("alignment", &DbModel::ArraySizes::alignment));
}

inline auto pointerishsize() {
  return make_table(
      "pointerishsize",
      make_column("id", &DbModel::PointerishSize::id, primary_key()),
      make_column("size", &DbModel::PointerishSize::size),
      make_column("alignment", &DbModel::PointerishSize::alignment));
}
// clang-format on

} // namespace TypeTableFn

#endif // _TABLE_DEFS_TYPE_H_
