#ifndef _DATABASE_WORKER_H_
#define _DATABASE_WORKER_H_

#include "model/config/configuration.h"
#include "model/sql/sql_model.h"
#include "util/thread_safe_queue.h"
#include <sqlite3.h>
#include <thread>

class DatabaseWorker {
private:
  ThreadSafeQueue<std::unique_ptr<SQLModel>> &queue_;
  sqlite3 *db_;
  std::thread worker_thread_;
  int batch_size_;

  void run();
  void processBatch(std::vector<std::unique_ptr<SQLModel>> &batch);
  void configureDatabase(const DatabaseConfig &config);

public:
  DatabaseWorker(ThreadSafeQueue<std::unique_ptr<SQLModel>> &queue,
                 const DatabaseConfig &config);
  ~DatabaseWorker() = default;
  void start();
  void stop();
};

#endif // _DATABASE_WORKER_H_
