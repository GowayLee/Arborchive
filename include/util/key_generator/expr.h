#ifndef _KEY_GENERATOR_EXPR_H_
#define _KEY_GENERATOR_EXPR_H_

#include "db/cache_repository.h"
#include "model/db/expr.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <string>

#define SEARCH_EXPR_CACHE(type)                                                \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Expr>>()                         \
      .find(type)

#define INSERT_EXPR_CACHE(key, id)                                             \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Expr>>()                         \
      .insert(key, id)

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Expr_ {
// For clang::Expr
KeyType makeKey(const Expr *expr, const ASTContext &ctx);

} // namespace Expr_

} // namespace KeyGen

#endif // _KEY_GENERATOR_EXPR_H_
