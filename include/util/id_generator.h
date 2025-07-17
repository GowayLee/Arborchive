#ifndef _ID_GENERATOR_H_
#define _ID_GENERATOR_H_

#include "util/logger/macros.h"
#include <atomic>
#include <mutex>
#include <type_traits>

#define GENID(type) IDGenerator::generateId<DbModel::type>()

class IDGenerator {
private:
  static std::mutex mutex_;
  static std::atomic<int> global_id_;

public:
  template <typename T> static int generateId() {
    static_assert(std::is_member_pointer<decltype(&T::id)>::value,
                  "Model must have an 'id' member");

    std::lock_guard<std::mutex> lock(mutex_);
    return ++global_id_;
  }

  template <typename T> static int getLastGeneratedId() {
    static_assert(std::is_member_pointer<decltype(&T::id)>::value,
                  "Model must have an 'id' member");

    std::lock_guard<std::mutex> lock(mutex_);
    int current_id = global_id_.load();
    return current_id > 0 ? current_id : 0;
  }

  template <typename T> static void generateAndSetId(const T &model) {
    model.id = generateId<T>();
  }
};
#endif // _ID_GENERATOR_H_
