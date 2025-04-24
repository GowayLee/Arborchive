#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "../third_party/sqlite_orm.h"
#include "model/config/configuration.h"
#include "table_init.h"
#include "util/logger/macros.h"
#include <filesystem>
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

    // 检查并清空现有数据库文件
    if (std::filesystem::exists(_sqliteDbPath)) {
      LOG_WARNING << "Database file already existed, try to remove..."
                  << std::endl;
      try {
        std::filesystem::remove(_sqliteDbPath);
      } catch (const std::filesystem::filesystem_error &e) {
        throw std::runtime_error("Failed to remove existing database file: " +
                                 std::string(e.what()));
      }
      LOG_INFO << "Remove old database file" << std::endl;
    }
    LOG_INFO << "Create new database file: " << _sqliteDbPath << std::endl;

    // 确保目录存在
    std::filesystem::path dbPath(_sqliteDbPath);
    if (!dbPath.parent_path().empty()) {
      std::filesystem::create_directories(dbPath.parent_path());
    }

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