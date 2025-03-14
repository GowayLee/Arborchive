#ifndef _DATABASE_WORKER_H_
#define _DATABASE_WORKER_H_

#include "model/config/configuration.h"
#include "model/sql/sql_model.h"
#include "util/thread_safe_queue.h"
#include <atomic>
#include <memory>
#include <sqlite3.h>
#include <thread>
#include <vector>

class DatabaseWorker {
public:
  DatabaseWorker(ThreadSafeQueue<std::unique_ptr<SQLModel>> &queue,
                 const DatabaseConfig &config);
  ~DatabaseWorker();

  void start();
  void stop();

  // 新增: 立即执行SQL的接口
  bool executeImmediate(const std::string &sql);
  // 新增: 获取最后插入ID的功能
  int64_t getLastInsertId();

private:
  void run();
  void processBatch(std::vector<std::unique_ptr<SQLModel>> &batch);
  bool initializeDatabase() const;
  void configureDatabase(const DatabaseConfig &config) const;

  ThreadSafeQueue<std::unique_ptr<SQLModel>> &queue_;
  sqlite3 *db_;
  std::thread worker_thread_;
  int batch_size_;
  std::atomic<bool> running_{false};
};

#endif // _DATABASE_WORKER_H_
