#ifndef _TABLE_DEFS_FUNCTION_H_
#define _TABLE_DEFS_FUNCTION_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/function.h"

using namespace sqlite_orm;

namespace FuncTableFn {

// clang-format off
inline auto functions() {
  return make_table(
      "functions",
      make_column("id", &DbModel::Function::id, primary_key()),
      make_column("name", &DbModel::Function::name),
      make_column("kind", &DbModel::Function::kind));
}

inline auto fun_decls() {
  return make_table(
      "fun_decls",
      make_column("id", &DbModel::FunDecl::id, primary_key()),
      make_column("function", &DbModel::FunDecl::function),
      make_column("type_id", &DbModel::FunDecl::type_id),
      make_column("name", &DbModel::FunDecl::name),
      make_column("location", &DbModel::FunDecl::location));
}

inline auto fun_def() {
  return make_table(
      "fun_def",
      make_column("id", &DbModel::FunDef::id, primary_key()));
}

inline auto function_return_type() {
  return make_table(
      "function_return_type",
      make_column("id", &DbModel::FuncRetType::id),
      make_column("return_type", &DbModel::FuncRetType::return_type));
}

inline auto pure_functions() {
  return make_table(
      "purefunctions",
      make_column("id", &DbModel::PureFuncs::id));
}

inline auto function_deleted() {
  return make_table(
      "function_deleted",
      make_column("id", &DbModel::FuncDeleted::id));
}

inline auto function_defaulted() {
  return make_table(
      "function_defaulted",
      make_column("id", &DbModel::FuncDefaulted::id));
}

inline auto function_prototyped() {
  return make_table(
      "function_prototyped",
      make_column("id", &DbModel::FuncPrototyped::id));
}

inline auto fun_specialized() {
  return make_table(
      "fun_specialized",
      make_column("id", &DbModel::FunSpecialized::id));
}

inline auto fun_implicit() {
  return make_table(
      "fun_implicit",
      make_column("id", &DbModel::FunImplicit::id));
}

inline auto function_entry_point() {
  return make_table(
      "function_entry_point",
      make_column("id", &DbModel::FuncEntryPt::id),
      make_column("entry_point", &DbModel::FuncEntryPt::entry_point));
}

inline auto deduction_guide_for_class() {
  return make_table(
      "deduction_guide_for_class",
      make_column("id", &DbModel::DeductionGuideForClass::id),
      make_column("class_template", &DbModel::DeductionGuideForClass::class_template));
}

inline auto fun_decl_throws() {
  return make_table(
      "fun_decl_throws",
      make_column("fun_decl", &DbModel::FunDeclThrow::fun_decl),
      make_column("index", &DbModel::FunDeclThrow::index),
      make_column("type_id", &DbModel::FunDeclThrow::type_id));
}

inline auto fun_decl_empty_throws() {
  return make_table(
      "fun_decl_empty_throws",
      make_column("fun_decl", &DbModel::FunDeclEmptyThrow::fun_decl));
}

inline auto fun_decl_noexcept() {
  return make_table(
      "fun_decl_noexcept",
      make_column("fun_decl", &DbModel::FunDeclNoexcept::fun_decl),
      make_column("constant", &DbModel::FunDeclNoexcept::constant));
}

inline auto fun_decl_empty_noexcept() {
  return make_table(
      "fun_decl_empty_noexcept",
      make_column("fun_decl", &DbModel::FunDeclEmptyNoexcept::fun_decl));
}

inline auto fun_decl_typedef_type() {
  return make_table(
      "fun_decl_typedef_type",
      make_column("fun_decl", &DbModel::FunDeclTypedefType::fun_decl),
      make_column("typedeftype_id", &DbModel::FunDeclTypedefType::tyepdeftype_id));
}

inline auto coroutine() {
  return make_table(
      "coroutine",
      make_column("function", &DbModel::Coroutine::function),
      make_column("traits", &DbModel::Coroutine::traits));
}

inline auto coroutine_new() {
  return make_table(
      "coroutine_new",
      make_column("function", &DbModel::CoroutineNew::function),
      make_column("new", &DbModel::CoroutineNew::new_));
}

inline auto coroutine_delete() {
  return make_table(
      "coroutine_delete",
      make_column("function", &DbModel::CoroutineDelete::function),
      make_column("delete", &DbModel::CoroutineDelete::delete_));
}

// clang-format on

} // namespace FuncTableFn

#endif // _TABLE_DEFS_FUNCTION_H_
