#ifndef _STORAGE_FACADE_H_
#define _STORAGE_FACADE_H_

#include "model/config/configuration.h"
#include "storage.h"

class StorageFacade {
public:
  static void initOrm(const DatabaseConfig config) {
    Storage::getInstance().initialize(config);
  }

  template <typename T> static inline void insertClassObj(T &&obj) {
    auto storage = Storage::getInstance().getStorage();
    auto statement = storage->prepare(replace(std::forward<T>(obj)));
    storage->execute(statement);
  }

  static inline void transaction(const std::function<bool()> &f) {
    auto storage = Storage::getInstance().getStorage();
    storage->transaction(f);
  }
};

#endif // _STORAGE_FACADE_H_