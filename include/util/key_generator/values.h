#ifndef _KEY_GENERATOR_VALUES_H_
#define _KEY_GENERATOR_VALUES_H_

#include "db/cache_repository.h"
#include "model/db/expr.h"
#include <clang/AST/ASTContext.h>
#include <string>

#define SEARCH_VALUES_CACHE(key)                                               \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Values>>()                       \
      .find(key)

#define INSERT_VALUES_CACHE(key, id)                                          \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Values>>()                       \
      .insert(key, id)

#define SEARCH_VALUETEXT_CACHE(key)                                           \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::ValueText>>()                    \
      .find(key)

#define INSERT_VALUETEXT_CACHE(key, id)                                       \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::ValueText>>()                    \
      .insert(key, id)

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Values {
// For literal values
KeyType makeKey(const std::string &value);
} // namespace Values

namespace ValueText {
// For literal text content
KeyType makeKey(const std::string &text);
} // namespace ValueText

} // namespace KeyGen

#endif // _KEY_GENERATOR_VALUES_H_