#include "db/async_manager.h"
#include "db/table_defines.h"
#include "model/config/configuration.h"
#include "util/logger/macros.h"
#include <filesystem>
#include <sqlite3.h>

AsyncDatabaseManager::AsyncDatabaseManager() {}

void AsyncDatabaseManager::loadConfig(const DatabaseConfig &config) {
  config_ = config;
  db_ = nullptr;
}

AsyncDatabaseManager &AsyncDatabaseManager::getInstance() {
  static AsyncDatabaseManager instance;
  return instance;
}

void AsyncDatabaseManager::pushModel(const std::string &sql) {
  queue_.push({sql, sequence_counter_++});
}

void AsyncDatabaseManager::start() {
  if (worker_thread_.joinable()) {
    throw std::runtime_error("Worker thread already running");
  }

  // 解析路径并创建目录
  std::filesystem::path dbFilePath(config_.path);
  std::filesystem::path dbDir = dbFilePath.parent_path();
  if (!dbDir.empty() && !std::filesystem::exists(dbDir))
    std::filesystem::create_directories(dbDir);

  if (sqlite3_open(config_.path.c_str(), &db_) != SQLITE_OK) {
    LOG_ERROR << "Failed to open database: " << config_.path << std::endl;
    throw std::runtime_error("Failed to open database");
  }
  // 检查数据库文件是否存在
  if (std::filesystem::exists(dbFilePath)) {
    LOG_WARNING << "Database already exists: " << config_.path
                << ". All existing data will be cleared!" << std::endl;
    clearDatabase();
  } else
    LOG_INFO << "Database file does not exist, creating new database: "
             << config_.path << std::endl;

  // 配置数据库参数
  configureDatabase();

  // 初始化数据库表
  if (!initializeDatabase()) {
    sqlite3_close(db_);
    throw std::runtime_error("Failed to initialize database");
  }

  worker_thread_ = std::thread(&AsyncDatabaseManager::workerLoop, this);
}

void AsyncDatabaseManager::stop() {
  queue_.stop();
  if (worker_thread_.joinable())
    worker_thread_.join();
}

void AsyncDatabaseManager::flush() {
  // 停止队列以处理所有待处理SQL
  queue_.stop();

  // 等待工作线程完成
  if (worker_thread_.joinable())
    worker_thread_.join();
  // 恢复队列状态
  queue_.resume();

  // 重新启动工作线程
  worker_thread_ = std::thread(&AsyncDatabaseManager::workerLoop, this);
}

void AsyncDatabaseManager::clearDatabase() {
  // {
  //   // 清除sqlite_sequence表的内容（如果存在）
  //   char *errMsg = nullptr;
  //   int rc = sqlite3_exec(db_,
  //                         "SELECT name FROM sqlite_master WHERE type='table'
  //                         " "AND name='sqlite_sequence'", nullptr, nullptr,
  //                         &errMsg);

  //   if (rc == SQLITE_OK) {
  //     if (!executeImmediate("DELETE FROM sqlite_sequence")) {
  //       throw std::runtime_error("Failed to clear sqlite_sequence");
  //     }
  //   } else if (errMsg) {
  //     sqlite3_free(errMsg);
  //   }
  // }

  // 清除所有用户表
  std::vector<std::string> tables;
  char *errMsg = nullptr;

  int rc = sqlite3_exec(
      db_,
      "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE "
      "'sqlite_%'",
      [](void *data, int argc, char **argv, char **colName) -> int {
        (void)argc;
        (void)colName;
        auto *tables = static_cast<std::vector<std::string> *>(data);
        tables->push_back(argv[0]);
        return 0;
      },
      &tables, &errMsg);

  if (rc != SQLITE_OK) {
    LOG_ERROR << "Failed to list tables: " << errMsg;
    sqlite3_free(errMsg);
    throw std::runtime_error("Failed to list database tables");
  }

  for (const auto &table : tables) {
    std::string dropSql = "DROP TABLE " + table;
    if (!executeImmediate(dropSql)) {
      throw std::runtime_error("Failed to clear existing database");
    }
  }

  LOG_INFO << "Existing database cleared successfully: " << config_.path
           << std::endl;
}

void AsyncDatabaseManager::workerLoop() {
  std::vector<QueueItem> batch;
  batch.reserve(config_.batch_size);

  try {
    while (!queue_.isStopped() || !queue_.empty()) {
      // 收集批量数据
      while (batch.size() < config_.batch_size) {
        try {
          batch.push_back(queue_.pop());
        } catch (const std::runtime_error &e) {
          // 队列已停止且为空
          break;
        }
      }

      if (!batch.empty()) {
        processBatch(batch);
        batch.clear();
      }
    }

    // 处理停止后剩余的队列数据
    while (!queue_.empty()) {
      try {
        batch.push_back(queue_.pop());
        if (batch.size() >= config_.batch_size) {
          processBatch(batch);
          batch.clear();
        }
      } catch (const std::runtime_error &e) {
        break;
      }
    }

    if (!batch.empty()) {
      processBatch(batch);
    }
  } catch (...) {
    // 处理任何未预期的异常
    if (!batch.empty()) {
      processBatch(batch);
    }
    throw;
  }
}

bool AsyncDatabaseManager::executeImmediate(const std::string &sql) {
  if (db_ == nullptr) {
    LOG_ERROR << "Databse has not been open: " << config_.path << std::endl;
    return false;
  }
  LOG_DEBUG << "Executing SQL: " << sql << std::endl;

  char *errMsg = nullptr;
  int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
  bool success = (rc == SQLITE_OK);

  if (!success) {
    LOG_ERROR << "SQL execution failed: " << errMsg << std::endl;
    sqlite3_free(errMsg);
  }

  return success;
}

void AsyncDatabaseManager::configureDatabase() {
  std::string cache_size_pragma =
      "PRAGMA cache_size=-" + std::to_string(config_.cache_size_mb * 1024);
  LOG_DEBUG << "Setting cache size: " << cache_size_pragma << std::endl;
  sqlite3_exec(db_, cache_size_pragma.c_str(), nullptr, nullptr, nullptr);

  std::string journal_mode_pragma =
      "PRAGMA journal_mode=" + config_.journal_mode;
  LOG_DEBUG << "Setting journal mode: " << journal_mode_pragma << std::endl;
  sqlite3_exec(db_, journal_mode_pragma.c_str(), nullptr, nullptr, nullptr);

  std::string synchronous_pragma = "PRAGMA synchronous=" + config_.synchronous;
  LOG_DEBUG << "Setting synchronous mode: " << synchronous_pragma << std::endl;
  sqlite3_exec(db_, synchronous_pragma.c_str(), nullptr, nullptr, nullptr);
}

bool AsyncDatabaseManager::initializeDatabase() {
  LOG_DEBUG << "Initializing database" << std::endl;

  const std::vector<std::pair<std::string, TableCreationFunc>>
      &tableCreationFuncs = TableRegistry::instance().getTables();

  LOG_DEBUG << "Table creation function number: " << tableCreationFuncs.size()
            << std::endl;

  if (tableCreationFuncs.empty()) {
    LOG_WARNING << "No table creation functions registered!" << std::endl;
  }

  char *errMsg = nullptr;

  for (const auto &[tableName, func] : tableCreationFuncs) {
    LOG_DEBUG << "Creating table: " << tableName << std::endl;

    std::string sql = func();
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
      LOG_ERROR << "Error in creating table " << tableName << ": " << errMsg
                << std::endl;
      sqlite3_free(errMsg);
      return false;
    }
  }
  return true;
}

void AsyncDatabaseManager::processBatch(std::vector<QueueItem> &batch) {
  if (!db_) {
    LOG_ERROR << "Database connection is not initialized" << std::endl;
    return;
  }

  char *errMsg = nullptr;
  if (sqlite3_exec(db_, "BEGIN TRANSACTION", nullptr, nullptr, &errMsg) !=
      SQLITE_OK) {
    LOG_ERROR << "Failed to begin transaction: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return;
  }

  for (const auto &item : batch) {
    LOG_DEBUG << "Executing SQL: " << item.sql << std::endl;
    if (sqlite3_exec(db_, item.sql.c_str(), nullptr, nullptr, &errMsg) !=
        SQLITE_OK) {
      LOG_ERROR << "SQL execution failed: " << errMsg << std::endl;
      sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
      sqlite3_free(errMsg);
      return;
    }
  }

  if (sqlite3_exec(db_, "COMMIT", nullptr, nullptr, &errMsg) != SQLITE_OK) {
    LOG_ERROR << "Failed to commit transaction: " << errMsg << std::endl;
    sqlite3_free(errMsg);
  } else {
    LOG_INFO << "Successfully committed batch of " << batch.size() << " items"
             << std::endl;
  }
}
