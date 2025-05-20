#ifndef _TABLE_INIT_H_
#define _TABLE_INIT_H_

#include "db/table_defs/variable.h"
#include "table_defs/compilation.h"
#include "table_defs/container.h"
#include "table_defs/declaration.h"
#include "table_defs/element.h"
#include "table_defs/expr.h"
#include "table_defs/function.h"
#include "table_defs/location.h"
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
      VarTableFn::variable(),
      VarTableFn::localscopevariable(),
      VarTableFn::localvariables(),
      VarTableFn::params(),
      VarTableFn::globalvariables(),
      VarTableFn::membervariables(),
      VarTableFn::var_decls(),
      // Type Tables
      TypeTableFn::types(),
      TypeTableFn::usertypes(),
      // Stmt Tables
      StmtTableFn::stmts(),
      // Expr Tables;
      ExprTableFn::exprs(),
      // Element Tables;
      ElementTableFn::parameterized_element()
    );
  // clang-format on
}

#endif // _TABLE_INIT_H_
