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

bool DatabaseManager::executeImmediate(const std::string &sql) {
  if (!worker_) {
    throw std::runtime_error("Database worker not initialized");
  }
  return worker_->executeImmediate(sql);
}

int64_t DatabaseManager::getLastInsertId() {
  if (!worker_) {
    throw std::runtime_error("Database worker not initialized");
  }
  return worker_->getLastInsertId();
}

void DatabaseManager::start() { worker_->start(); }

void DatabaseManager::stop() { worker_->stop(); }
