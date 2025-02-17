#include "db/manager.h"

DatabaseManager::DatabaseManager(const DatabaseConfig &config)
    : worker_(std::make_unique<DatabaseWorker>(queue_, config)) {}

DatabaseManager &DatabaseManager::getInstance(const DatabaseConfig &config) {
  static DatabaseManager instance(config);
  return instance;
}

void DatabaseManager::pushModel(std::unique_ptr<SQLModel> model) {
  queue_.push(std::move(model));
}

void DatabaseManager::start() { worker_->start(); }

void DatabaseManager::stop() { worker_->stop(); }
