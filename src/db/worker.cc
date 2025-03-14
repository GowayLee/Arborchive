#include "db/worker.h"
#include "db/table_defines.h"
#include "model/config/configuration.h"
#include "util/logger/macros.h"
#include <filesystem>
#include <iostream>
#include <stdexcept>

DatabaseWorker::DatabaseWorker(
    ThreadSafeQueue<std::unique_ptr<SQLModel>> &queue,
    const DatabaseConfig &config)
    : queue_(queue), batch_size_(config.batch_size) {

  // 解析路径并创建目录
  std::filesystem::path dbFilePath(config.path);
  std::filesystem::path dbDir = dbFilePath.parent_path();
  if (!dbDir.empty() && !std::filesystem::exists(dbDir))
    std::filesystem::create_directories(dbDir);

  // 检查数据库文件是否存在
  if (!std::filesystem::exists(dbFilePath)) {
    LOG_INFO << "Database file does not exist, creating new database: "
             << config.path << std::endl;
  }

  if (sqlite3_open(config.path.c_str(), &db_) != SQLITE_OK) {
    LOG_ERROR << "Failed to open database: " << config.path << std::endl;
    throw std::runtime_error("Failed to open database");
  }
  if (std::filesystem::exists(dbFilePath)) {
    LOG_WARNING << "Database already exists: " << config.path
                << ". All existing data will be cleared!" << std::endl;

    // 清除所有用户表
    std::vector<std::string> tables;
    sqlite3_exec(
        db_,
        "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE "
        "'sqlite_%'",
        [](void *data, int argc, char **argv, char **colName) -> int {
          auto *tables = static_cast<std::vector<std::string> *>(data);
          tables->push_back(argv[0]);
          return 0;
        },
        &tables, nullptr);

    for (const auto &table : tables) {
      std::string dropSql = "DROP TABLE " + table;
      if (!executeImmediate(dropSql)) {
        throw std::runtime_error("Failed to clear existing database");
      }
    }

    // 清除sqlite_sequence表的内容
    if (!executeImmediate("DELETE FROM sqlite_sequence")) {
      throw std::runtime_error("Failed to clear sqlite_sequence");
    }
    LOG_INFO << "Existing database cleared successfully: " << config.path
             << std::endl;
  } else {
    LOG_INFO << "New database created successfully: " << config.path
             << std::endl;
  }

  configureDatabase(config);
  initializeDatabase();
}
DatabaseWorker::~DatabaseWorker() {
  if (running_.load())
    stop();
}

bool DatabaseWorker::executeImmediate(const std::string &sql) {
  char *errMsg = nullptr;
  int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
  if (rc != SQLITE_OK) {
    LOG_ERROR << "Immediate execution failed: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

int64_t DatabaseWorker::getLastInsertId() {
  return sqlite3_last_insert_rowid(db_);
}

void DatabaseWorker::configureDatabase(const DatabaseConfig &config) const {
  std::string cache_size_pragma =
      "PRAGMA cache_size=-" + std::to_string(config.cache_size_mb * 1024);
  LOG_DEBUG << "Setting cache size: " << cache_size_pragma << std::endl;
  sqlite3_exec(db_, cache_size_pragma.c_str(), nullptr, nullptr, nullptr);

  std::string journal_mode_pragma =
      "PRAGMA journal_mode=" + config.journal_mode;
  LOG_DEBUG << "Setting journal mode: " << journal_mode_pragma << std::endl;
  sqlite3_exec(db_, journal_mode_pragma.c_str(), nullptr, nullptr, nullptr);

  std::string synchronous_pragma = "PRAGMA synchronous=" + config.synchronous;
  LOG_DEBUG << "Setting synchronous mode: " << synchronous_pragma << std::endl;
  sqlite3_exec(db_, synchronous_pragma.c_str(), nullptr, nullptr, nullptr);
}

bool DatabaseWorker::initializeDatabase() const {
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
      sqlite3_close(db_);
      return false;
    }
  }
  return true;
}

void DatabaseWorker::start() {
  LOG_INFO << "Starting database worker thread" << std::endl;
  running_.store(true);
  worker_thread_ = std::thread(&DatabaseWorker::run, this);
}

void DatabaseWorker::stop() {
  LOG_INFO << "Stopping database worker thread" << std::endl;
  // 先设置停止标志并停止队列
  running_.store(false);
  queue_.stop();

  // 等待工作线程处理完剩余数据
  if (worker_thread_.joinable()) {
    if (worker_thread_.get_id() == std::thread::id()) {
      std::cerr << "Error: worker_thread_ ID is invalid!" << std::endl;
      return;
      worker_thread_.join();
    }
  }

  // 关闭数据库连接
  if (db_) {
    LOG_INFO << "Closing database connection..." << std::endl;

    // 关闭数据库连接
    int rc = sqlite3_close(db_);
    if (rc != SQLITE_OK) {
      LOG_ERROR << "Failed to close database connection: "
                << sqlite3_errmsg(db_) << std::endl;
    } else {
      LOG_INFO << "Database connection closed successfully" << std::endl;
    }
    db_ = nullptr;
  }
}

void DatabaseWorker::run() {
  std::vector<std::unique_ptr<SQLModel>> batch;
  while (running_.load() || !queue_.empty()) {
    batch.push_back(std::move(queue_.pop()));
    if (batch.size() >= static_cast<std::size_t>(batch_size_)) {
      processBatch(batch);
      batch.clear();
    }
  }
  if (!batch.empty())
    processBatch(batch);
}

void DatabaseWorker::processBatch(
    std::vector<std::unique_ptr<SQLModel>> &batch) {
  char *errMsg = nullptr;
  int rc = sqlite3_exec(db_, "BEGIN TRANSACTION", nullptr, nullptr, &errMsg);
  if (rc != SQLITE_OK) {
    LOG_ERROR << "Failed to begin transaction: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return;
  }

  try {
    for (const auto &model : batch) {
      if (!model) {
        std::cout << "Null model in batch, skipping" << std::endl;
        continue;
      }
      std::string sql = model->serialize();
      LOG_DEBUG << "Executing SQL: " << sql << std::endl;
      rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
      if (rc != SQLITE_OK) {
        LOG_ERROR << "SQL execution failed: " << errMsg << std::endl;
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        sqlite3_free(errMsg);
        throw std::runtime_error("SQL execution failed");
      }
    }

    rc = sqlite3_exec(db_, "COMMIT", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
      LOG_ERROR << "Failed to commit transaction: " << errMsg << std::endl;
      sqlite3_free(errMsg);
      throw std::runtime_error("Failed to commit transaction");
    } else {
      std::cout << "Transaction committed successfully" << std::endl;
    }
  } catch (const std::exception &e) {
    LOG_ERROR << "Error processing batch: " << e.what() << std::endl;
    batch.clear();
    LOG_DEBUG << "Batch cleared after error" << std::endl;
    throw;
  }
  batch.clear();
  std::cout << "Batch cleared successfully" << std::endl;
}