#ifndef _DATABASE_MANAGER_H_
#define _DATABASE_MANAGER_H_

#include "model/config/configuration.h"
#include "model/sql/sql_model.h"
#include "util/thread_safe_queue.h"
#include "worker.h"
#include <memory>

class DatabaseManager {
private:
  ThreadSafeQueue<std::unique_ptr<SQLModel>> queue_;
  std::unique_ptr<DatabaseWorker> worker_;

  DatabaseManager(const DatabaseConfig &config);
  DatabaseManager(const DatabaseManager &) = delete;
  DatabaseManager &operator=(const DatabaseManager &) = delete;

public:
  static DatabaseManager &getInstance(const DatabaseConfig &config);
  void pushModel(std::unique_ptr<SQLModel> model);
  void start();
  void stop();
};

#endif // _DATABASE_MANAGER_H_