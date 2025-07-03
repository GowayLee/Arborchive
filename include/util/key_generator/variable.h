#ifndef _KEY_GENERATOR_VARIABLE_H_
#define _KEY_GENERATOR_VARIABLE_H_

#include "db/cache_repository.h"
#include "model/db/variable.h"
#include <clang/AST/ASTContext.h>
#include <string>

#define SEARCH_VARIABLE_CACHE(type)                                            \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Variable>>()                     \
      .find(type)

#define INSERT_VARIABLE_CACHE(key, id)                                         \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Variable>>()                     \
      .insert(key, id)

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Var {
// For clang::VarDecl
KeyType makeKey(const VarDecl *VD, const ASTContext &ctx);
} // namespace Var

} // namespace KeyGen

#endif // _KEY_GENERATOR_VARIABLE_H_
