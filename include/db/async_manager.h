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
    std::shared_ptr<SQLModel> model;
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

  void pushModel(std::shared_ptr<SQLModel> model);
  void start();
  void stop();

  bool executeImmediate(const std::string &sql);

  // 新增查询接口
  template <typename T = SQLModel>
  std::vector<std::unique_ptr<T>> queryModels(const std::string &sql) {
    std::vector<std::unique_ptr<T>> results;
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto model = std::make_unique<T>();
        int col_count = sqlite3_column_count(stmt);
        for (int i = 0; i < col_count; ++i) {
          const char *col_name = sqlite3_column_name(stmt, i);
          const char *col_value =
              reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
          if (col_name && col_value)
            model->setField(col_name, std::string(col_value));
        }
        results.push_back(model);
      }
      sqlite3_finalize(stmt);
    } else
      LOG_ERROR << "SQL execution failed: " << sql << std::endl;
    return results;
  }
};

#endif // _DATABASE_ASYNC_MANAGER_H_
