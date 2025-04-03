#ifndef _ID_GENERATOR_H_
#define _ID_GENERATOR_H_

#include "util/logger/macros.h"
#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>

class IDGenerator {
private:
  static std::mutex mutex_;
  static std::unordered_map<std::string, std::atomic<uint64_t>> table_ids_;

public:
  static uint64_t generateId(const std::string &table_name) {
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

  static uint64_t getLastGeneratedId(const std::string &table_name) {
    try {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = table_ids_.find(table_name);
      if (it == table_ids_.end()) {
        LOG_WARNING << "No IDs generated yet for table: " << table_name
                    << std::endl;
        return 0;
      }
      uint64_t last_id = it->second.load();
      return last_id > 0 ? last_id - 1 : 0;
    } catch (const std::exception &e) {
      LOG_ERROR << "Failed to get last ID for table " << table_name << ": "
                << e.what() << std::endl;
      throw;
    }
  }
};
#endif // _ID_GENERATOR_H_
