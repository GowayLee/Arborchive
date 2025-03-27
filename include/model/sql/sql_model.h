#ifndef _SQL_MODEL_H_
#define _SQL_MODEL_H_

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

class SQLModel {
public:
  virtual ~SQLModel() = default;
  virtual std::string getTableName() const = 0;
  virtual std::string serialize() const = 0;

  // 生成表特定的唯一ID
  static uint64_t generateId(const std::string &table_name) {
    static std::mutex mutex_;
    static std::unordered_map<std::string, std::atomic<uint64_t>> table_ids_;

    std::lock_guard<std::mutex> lock(mutex_);
    auto &id_generator = table_ids_[table_name];
    if (id_generator == 0)
      id_generator = 1; // 初始化第一个ID
    return id_generator.fetch_add(1);
  }

  template <typename T> void setField(const std::string &name, const T &value) {
    fields_[name] = std::to_string(value);
  }

  // Specialization for string type
  void setField(const std::string &name, const std::string &value) {
    fields_[name] = "'" + value + "'"; // Add quotes for strings
  }

protected:
  std::map<std::string, std::string> fields_;
};

#endif // _SQL_MODEL_H_
