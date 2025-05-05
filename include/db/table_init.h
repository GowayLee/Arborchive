#ifndef _TABLE_INIT_H_
#define _TABLE_INIT_H_

#include "table_defs/compilation.h"
#include "table_defs/container.h"
#include "table_defs/declaration.h"
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
      // Type Tables
      TypeTableFn::types(),
      TypeTableFn::usertypes(),
      // Stmt Tables
      StmtTableFn::stmts()
    );
  // clang-format on
}

#endif // _TABLE_INIT_H_
