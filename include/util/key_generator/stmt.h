#ifndef _KEY_GENERATOR_STMT_H_
#define _KEY_GENERATOR_STMT_H_

#include "db/cache_repository.h"
#include "model/db/stmt.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <string>

#define SEARCH_STMT_CACHE(type)                                                \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Stmt>>()                         \
      .find(type)

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Stmt_ {
// For clang::Stmt
KeyType makeKey(const Stmt *stmt, const ASTContext &ctx);
} // namespace Stmt_

} // namespace KeyGen

#endif // _KEY_GENERATOR_STMT_H_
