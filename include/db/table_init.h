#ifndef _TABLE_INIT_H_
#define _TABLE_INIT_H_

#include "db/table_defs/class.h"
#include "db/table_defs/variable.h"
#include "table_defs/compilation.h"
#include "table_defs/container.h"
#include "table_defs/declaration.h"
#include "table_defs/element.h"
#include "table_defs/expr.h"
#include "table_defs/function.h"
#include "table_defs/location.h"
#include "table_defs/specifiers.h"
#include "table_defs/stmt.h"
#include "table_defs/type.h"

using namespace sqlite_orm;

// Definition of function used to delcare all the table existed
inline auto initStorage(const std::string &path) {
  // clang-format off
  return make_storage(
      path,
      // Compilation Tables
      CompTableFn::compilations(),
      CompTableFn::compilatio_args(),
      CompTableFn::compilatio_build_mode(),
      CompTableFn::compilatio_time(),
      CompTableFn::compilation_finished(),
      // Location Tables
      LocTableFn::locations(),
      LocTableFn::locations_default(),
      LocTableFn::locations_stmt(),
      LocTableFn::locations_expr(),
      // Container Tables
      ContainerTableFn::container(),
      ContainerTableFn::files(),
      ContainerTableFn::folders(),
      ContainerTableFn::namespaces(),
      ContainerTableFn::namespace_inline(),
      ContainerTableFn::namespacembrs(),
      // Declaration Tables
      DeclTableFn::declarations(),
      // Function Tables
      FuncTableFn::functions(),
      FuncTableFn::fun_decls(),
      FuncTableFn::fun_def(),
      FuncTableFn::function_return_type(),
      FuncTableFn::pure_functions(),
      FuncTableFn::function_deleted(),
      FuncTableFn::function_defaulted(),
      FuncTableFn::function_prototyped(),
      FuncTableFn::fun_specialized(),
      FuncTableFn::fun_implicit(),
      FuncTableFn::function_entry_point(),
      FuncTableFn::deduction_guide_for_class(),
      FuncTableFn::fun_decl_throws(),
      FuncTableFn::fun_decl_empty_throws(),
      FuncTableFn::fun_decl_noexcept(),
      FuncTableFn::fun_decl_empty_noexcept(),
      FuncTableFn::fun_decl_typedef_type(),
      FuncTableFn::coroutine(),
      FuncTableFn::coroutine_new(),
      FuncTableFn::coroutine_delete(),
      // Variable Tables
      // TO BE REMOVED: Variable and LocalScopeVar tables - intermediary tables no longer needed
      VarTableFn::variable(), // deprecated - intermediary table no longer needed
      // VarTableFn::localscopevariable(),
      VarTableFn::localvariables(),
      VarTableFn::params(),
      VarTableFn::globalvariables(),
      VarTableFn::membervariables(),
      VarTableFn::var_decls(),
      VarTableFn::var_def(),
      VarTableFn::var_specialized(),
      VarTableFn::var_decl_specifiers(),
      VarTableFn::is_structured_binding(),
      VarTableFn::var_requires(),
      // Type Tables
      TypeTableFn::types(), // deprecated - intermediary table no longer needed
      TypeTableFn::type_decls(),
      TypeTableFn::type_def(),
      TypeTableFn::type_decl_top(),
      TypeTableFn::builtintypes(),
      TypeTableFn::derivedtypes(),
      TypeTableFn::usertypes(),
      TypeTableFn::routinetypes(),
      TypeTableFn::routinetypeargs(),
      TypeTableFn::ptrtomembers(),
      TypeTableFn::decltypes(),
      TypeTableFn::is_pod_class(),
      TypeTableFn::is_standard_layout_class(),
      TypeTableFn::is_complete(),
      TypeTableFn::enumconstants(),
      TypeTableFn::typedefbase(),
      TypeTableFn::arraysizes(),
      TypeTableFn::pointerishsize(),
      // Stmt Tables
      StmtTableFn::stmts(),
      StmtTableFn::if_initalization(),
      StmtTableFn::if_then(),
      StmtTableFn::if_else(),
      // StmtTableFn::stmt_for_or_range_based_for(),
      StmtTableFn::for_initialization(),
      StmtTableFn::for_condition(),
      StmtTableFn::for_update(),
      StmtTableFn::for_body(),
      StmtTableFn::while_body(),
      StmtTableFn::do_body(),
      StmtTableFn::switch_body(),
      StmtTableFn::switch_initialization(),
      StmtTableFn::switch_case(),
      // Expr Tables;
      ExprTableFn::exprs(),
      ExprTableFn::funbind(),
      ExprTableFn::iscall(),
      ExprTableFn::varbind(),
      ExprTableFn::values(),
      ExprTableFn::valuetext(),
      ExprTableFn::valuebind(),
      ExprTableFn::aggregatearrayinit(),
      ExprTableFn::aggregatefieldinit(),
      ExprTableFn::sizeofbind(),
      // Element Tables;
      ElementTableFn::parameterized_element(),
      // Class Table;
      ClassTableFn::memeber(),
      // Specifier Tables
      SpecifierTableFn::specifiers(),
      SpecifierTableFn::typespecifiers(),
      SpecifierTableFn::funspecifiers(),
      SpecifierTableFn::varspecifiers()
    );
  // clang-format on
}

#endif // _TABLE_INIT_H_
