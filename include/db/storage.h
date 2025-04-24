#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "../third_party/sqlite_orm.h"
#include "model/config/configuration.h"
#include "table_init.h"
#include <memory>

using namespace sqlite_orm;

class Storage {
public:
  using StorageType = decltype(initStorage(""));

  static Storage &getInstance() {
    static Storage instance;
    return instance;
  }

  // initialize ORM
  inline void initialize(const DatabaseConfig config) {
    if (!config.path.empty())
      _sqliteDbPath = config.path;

    _storage =
        std::make_unique<Storage::StorageType>(initStorage(_sqliteDbPath));
    _storage->sync_schema();
    _storage->pragma.synchronous(0); // PRAGMA synchronous = OFF
    _storage->pragma.journal_mode(
        journal_mode::MEMORY); // PRAGMA journal_mode = MEMORY
    _initialized = true;
  }

  // Check if all the models are completely mapped
  inline bool isInitialised() const { return _initialized.load(); }

  // Get Sqlite ORM Object
  std::shared_ptr<StorageType> getStorage() { return _storage; }

  ~Storage() = default;

  Storage(const Storage &) = delete;

  Storage &operator=(const Storage &) = delete;

private:
  Storage() = default;

  std::string _sqliteDbPath;
  std::atomic<bool> _initialized{false};
  std::shared_ptr<StorageType> _storage;
};

#endif // _STORAGE_H_