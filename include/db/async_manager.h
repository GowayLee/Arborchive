#ifndef _DATABASE_ASYNC_MANAGER_H_
#define _DATABASE_ASYNC_MANAGER_H_

#include "db/table_defines.h"
#include "model/config/configuration.h"
#include "model/sql/sql_model.h"
#include "util/thread_safe_queue.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <sqlite3.h>
#include <thread>

class AsyncDatabaseManager {
private:
  struct QueueItem {
    std::unique_ptr<SQLModel> model;
    uint64_t sequence_id; // 用于保证顺序
  };

  ThreadSafeQueue<QueueItem> queue_;
  std::atomic<uint64_t> sequence_counter_{0};

  DatabaseConfig config_;
  std::thread worker_thread_;
  sqlite3 *db_;

  void workerLoop();
  void configureDatabase();
  bool initializeDatabase();
  void processBatch(std::vector<QueueItem> &batch);

  void clearDatabase();

  // 单例模式实现
  AsyncDatabaseManager();
  AsyncDatabaseManager(const AsyncDatabaseManager &) = delete;
  AsyncDatabaseManager &operator=(const AsyncDatabaseManager &) = delete;

public:
  static AsyncDatabaseManager &getInstance();
  void loadConfig(const DatabaseConfig &config);

  void pushModel(std::unique_ptr<SQLModel> model);
  void start();
  void stop();

  bool executeImmediate(const std::string &sql);
  int64_t getLastInsertId();
};

#endif // _DATABASE_ASYNC_MANAGER_H_