#ifndef _KEY_GENERATOR_FUNCTION_H_
#define _KEY_GENERATOR_FUNCTION_H_

#include "db/cache_repository.h"
#include "model/db/function.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <string>

#define SEARCH_FUNCTION_CACHE(type)                                            \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Function>>()                     \
      .find(type)

#define INSERT_FUNCTION_CACHE(key, id)                                         \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Function>>()                     \
      .insert(key, id)

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Function {

KeyType makeKey(const FunctionDecl *FD, ASTContext *Context);

} // namespace Function

} // namespace KeyGen

#endif // _KEY_GENERATOR_FUNCTION_H_
