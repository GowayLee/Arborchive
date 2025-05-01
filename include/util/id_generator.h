#ifndef _ID_GENERATOR_H_
#define _ID_GENERATOR_H_

#include "util/logger/macros.h"
#include <atomic>
#include <mutex>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#define GENID(type) IDGenerator::generateId<DbModel::type>()

class IDGenerator {
private:
  static std::mutex mutex_;
  static std::unordered_map<std::type_index, std::atomic<int>> type_ids_;

  template <typename T> static std::type_index getTypeIndex() {
    return std::type_index(typeid(T));
  }

public:
  template <typename T> static int generateId() {
    static_assert(std::is_member_pointer<decltype(&T::id)>::value,
                  "Model must have an 'id' member");

    std::lock_guard<std::mutex> lock(mutex_);
    auto &id_generator = type_ids_[getTypeIndex<T>()];
    if (id_generator == 0) {
      LOG_INFO << "Initializing first ID for type: " << typeid(T).name()
               << std::endl;
      id_generator = 1;
    }
    int new_id = id_generator.fetch_add(1);
    // LOG_DEBUG << "Generated new ID " << new_id
    //           << " for type: " << typeid(T).name() << std::endl;
    return new_id;
  }

  template <typename T> static int getLastGeneratedId() {
    static_assert(std::is_member_pointer<decltype(&T::id)>::value,
                  "Model must have an 'id' member");

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = type_ids_.find(getTypeIndex<T>());
    if (it == type_ids_.end()) {
      LOG_WARNING << "No IDs generated yet for type: " << typeid(T).name()
                  << std::endl;
      return 0;
    }
    int last_id = it->second.load();
    return last_id > 0 ? last_id - 1 : 0;
  }

  template <typename T> static void generateAndSetId(const T &model) {
    model.id = generateId<T>();
  }
};
#endif // _ID_GENERATOR_H_
