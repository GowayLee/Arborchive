#ifndef _STORAGE_FACADE_H_
#define _STORAGE_FACADE_H_

#include "model/config/configuration.h"
#include <functional>
#include <memory>

#define STG StorageFacade::getInstance()

class StorageFacade {
public:
  static StorageFacade &getInstance() {
    static StorageFacade instance;
    return instance;
  }

  void initOrm(const DatabaseConfig config);

  template <typename T> void insertClassObj(T &&obj);

  void transaction(const std::function<bool()> &f);

  ~StorageFacade() = default;
  StorageFacade(const StorageFacade &) = delete;
  StorageFacade &operator=(const StorageFacade &) = delete;

private:
  StorageFacade() = default;
};

#endif // _STORAGE_FACADE_H_