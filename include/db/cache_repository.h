#ifndef _CACHE_REPOSITORY_H_
#define _CACHE_REPOSITORY_H_

#include <memory>
#include <optional>
#include <typeindex>
#include <unordered_map>

template <typename Model, typename KeyType = typename Model::KeyType,
          typename IdType = int>
class CacheRepository {
public:
  std::optional<IdType> find(const KeyType &key) const {
    if (auto it = cache_.find(key); it != cache_.end())
      return it->second;
    return std::nullopt;
  }

  IdType insert(const KeyType &key, IdType id) {
    return cache_.emplace(key, id).first->second;
  }

  size_t size() const { return cache_.size(); }

  void clear() { cache_.clear(); }

private:
  std::unordered_map<KeyType, IdType> cache_;
};

// Cache Manager Singleton
class CacheManager {
public:
  static CacheManager &instance() {
    static CacheManager instance;
    return instance;
  }

  template <typename RepoType> RepoType &getRepository() {
    // Try to get the repository from the map, if not found, create a new one
    auto [it, inserted] = repositories_.try_emplace(typeid(RepoType));
    if (inserted)
      it->second = std::make_unique<Holder<RepoType>>();

    return *static_cast<Holder<RepoType> *>(it->second.get())->repo;
  }

  void clearAll() { repositories_.clear(); }

private:
  // RepositoryBase Type
  struct RepositoryBase {
    virtual ~RepositoryBase() = default;
  };

  // Holder for RepositoryBase
  template <typename T> struct Holder : RepositoryBase {
    std::unique_ptr<T> repo = std::make_unique<T>();
  };

  std::unordered_map<std::type_index, std::unique_ptr<RepositoryBase>>
      repositories_;

  CacheManager() = default;
  ~CacheManager() = default;
};

#endif // _CACHE_REPOSITORY_H_
