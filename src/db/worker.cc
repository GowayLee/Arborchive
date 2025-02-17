#include "db/worker.h"
#include "model/config/configuration.h"
#include "util/logger/macros.h"
#include <stdexcept>

DatabaseWorker::DatabaseWorker(
    ThreadSafeQueue<std::unique_ptr<SQLModel>> &queue,
    const DatabaseConfig &config)
    : queue_(queue), batch_size_(config.batch_size) {
  if (sqlite3_open(config.path.c_str(), &db_) != SQLITE_OK) {
    LOG_ERROR << "Failed to open database: " << config.path << std::endl;
    throw std::runtime_error("Failed to open database");
  } else
    LOG_INFO << "Database opened successfully: " << config.path << std::endl;

  configureDatabase(config);
}

DatabaseWorker::~DatabaseWorker() {
  stop();
  sqlite3_close(db_);
}

void DatabaseWorker::configureDatabase(const DatabaseConfig &config) {
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

void DatabaseWorker::start() {
  LOG_INFO << "Starting database worker thread" << std::endl;
  worker_thread_ = std::thread(&DatabaseWorker::run, this);
}

void DatabaseWorker::stop() {
  LOG_INFO << "Stopping database worker thread" << std::endl;
  queue_.stop();
  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
}

void DatabaseWorker::run() {
  std::vector<std::unique_ptr<SQLModel>> batch;
  while (!queue_.isStopped() || !queue_.empty()) {
    try {
      batch.push_back(std::move(queue_.pop()));
      if (batch.size() >= batch_size_) {
        processBatch(batch);
        batch.clear();
      }
    } catch (const std::runtime_error &e) {
      break;
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

  for (const auto &model : batch) {
    std::string sql = model->serialize();
    LOG_DEBUG << "Executing SQL: " << sql << std::endl;
    rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
      LOG_ERROR << "SQL execution failed: " << errMsg << std::endl;
      sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
      sqlite3_free(errMsg);
      return;
    }
  }

  rc = sqlite3_exec(db_, "COMMIT", nullptr, nullptr, &errMsg);
  if (rc != SQLITE_OK) {
    LOG_ERROR << "Failed to commit transaction: " << errMsg << std::endl;
    sqlite3_free(errMsg);
  } else {
    LOG_INFO << "Transaction committed successfully" << std::endl;
  }
}
