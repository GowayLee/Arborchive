#include "db/manager.h"

DatabaseManager::DatabaseManager() {}

DatabaseManager &DatabaseManager::getInstance() {
  static DatabaseManager instance;
  return instance;
}

bool DatabaseManager::loadConfig(const DatabaseConfig &config) {
  worker_ = std::make_unique<DatabaseWorker>(queue_, config);
  return true;
}

void DatabaseManager::pushModel(std::unique_ptr<SQLModel> model) {
  queue_.push(std::move(model));
}

void DatabaseManager::start() { worker_->start(); }

void DatabaseManager::stop() { worker_->stop(); }
