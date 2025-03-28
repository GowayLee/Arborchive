#ifndef _ID_GENERATOR_H_
#define _ID_GENERATOR_H_

#include "util/logger/macros.h"
#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>

class IDGenerator {
public:
  // 生成表特定的唯一ID
  static uint64_t generateId(const std::string &table_name) {
    static std::mutex mutex_;
    static std::unordered_map<std::string, std::atomic<uint64_t>> table_ids_;

    try {
      std::lock_guard<std::mutex> lock(mutex_);
      auto &id_generator = table_ids_[table_name];
      if (id_generator == 0) {
        LOG_INFO << "Initializing first ID for table: " << table_name
                 << std::endl;
        id_generator = 1;
      }
      uint64_t new_id = id_generator.fetch_add(1);
      LOG_DEBUG << "Generated new ID " << new_id << " for table: " << table_name
                << std::endl;
      return new_id;
    } catch (const std::exception &e) {
      LOG_ERROR << "Failed to generate ID for table " << table_name << ": "
                << e.what() << std::endl;
      throw;
    }
  }

  // 获取最近生成的ID
  static uint64_t getLastGeneratedId(const std::string &table_name) {
    static std::mutex mutex_;
    static std::unordered_map<std::string, std::atomic<uint64_t>> table_ids_;

    try {
      std::lock_guard<std::mutex> lock(mutex_);

      // 检查表是否存在
      auto it = table_ids_.find(table_name);
      if (it == table_ids_.end()) {
        LOG_WARNING << "No IDs generated yet for table: " << table_name
                    << std::endl;
        return 0;
      }

      uint64_t last_id = it->second.load();
      if (last_id > 0) {
        last_id--;
      }

      LOG_DEBUG << "Retrieved last ID " << last_id
                << " for table: " << table_name << std::endl;
      return last_id;
    } catch (const std::exception &e) {
      LOG_ERROR << "Failed to get last ID for table " << table_name << ": "
                << e.what() << std::endl;
      throw;
    }
  }
};

#endif // _ID_GENERATOR_H_