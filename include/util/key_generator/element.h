#ifndef _KEY_GENERATOR_ELEMENT_H_
#define _KEY_GENERATOR_ELEMENT_H_

#include "db/cache_repository.h"
#include "model/db/element.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <string>

#define SEARCH_ELEMENT_CACHE(type)                                             \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::ParameterizedElement>>()         \
      .find(type)

#define INSERT_ELEMENT_CACHE(key, id)                                          \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::ParameterizedElement>>()         \
      .insert(key, id)

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Element {

KeyType makeKeyFromFuncKey(const KeyType funcKey);
KeyType makeKey(const FunctionDecl *FD, ASTContext *Context);

} // namespace Element

} // namespace KeyGen

#endif // _KEY_GENERATOR_ELEMENT_H_
