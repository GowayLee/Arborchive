#ifndef _TABLE_INIT_H_
#define _TABLE_INIT_H_

#include "table_defs/compilation.h"
#include "table_defs/container.h"
#include "table_defs/location.h"

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
      ContainerTableFn::folders()
    );
  // clang-format on
}

#endif // _TABLE_INIT_H_