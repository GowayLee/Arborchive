#ifndef _KEY_GENERATOR_PREPROCESSOR_H_
#define _KEY_GENERATOR_PREPROCESSOR_H_

#include "db/cache_repository.h"
#include "model/db/preprocessor.h"
#include <string>

#define SEARCH_PREPROC_CACHE(key)                                              \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Preprocdirect>>()                \
      .find(key)

#define INSERT_PREPROC_CACHE(key, id)                                          \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Preprocdirect>>()                \
      .insert(key, id)

using KeyType = std::string;

namespace KeyGen {

namespace Preprocessor {

// Generate unique key for preprocessor directive based on source location
KeyType makeKey(const std::string &filename, unsigned line, unsigned column,
                int directive_kind, const std::string &extra_tag = "");

} // namespace Preprocessor

} // namespace KeyGen

#endif // _KEY_GENERATOR_PREPROCESSOR_H_
