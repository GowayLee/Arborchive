#include "db/storage_facade.h"
#include "db/storage.h"

void StorageFacade::initOrm(const DatabaseConfig config) {
  Storage::getInstance().initialize(config);
}

template <typename T> void StorageFacade::insertClassObj(T &&obj) {
  auto storage = Storage::getInstance().getStorage();
  auto statement = storage->prepare(replace(std::forward<T>(obj)));
  storage->execute(statement);
}

void StorageFacade::transaction(const std::function<bool()> &f) {
  auto storage = Storage::getInstance().getStorage();
  storage->transaction(f);
}

// Instantiations of template methods defined here
#include "storage_facade_instantiations.inc"