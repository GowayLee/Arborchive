#ifndef _DB_DEPENDENCY_MANAGER_H_
#define _DB_DEPENDENCY_MANAGER_H_

#include <functional>
#include <string>
#include <vector>

using KeyType = std::string;

// 用于指定Key属于哪种类型的缓存，以便DependencyManager知道去哪里查找
enum class CacheType {
  FUNCTION,
  TYPE,
  USERTYPE,
  STMT,
  EXPR,
  VARIABLE,
  MEMBERVERY,
  ELEMENT
};

// 描述一个待处理的更新操作
struct PendingUpdate {
  KeyType dependencyKey;                // 依赖的节点的Key
  CacheType keyType;                    // Key的类型，用于指定搜索哪个缓存
  std::function<void(int)> updater;     // 获取ID后要执行的更新回调函数
};

class DependencyManager {
public:
  static DependencyManager &instance();

  // 添加一个新的待处理依赖
  void addDependency(const PendingUpdate &update);

  // 在AST遍历结束后，解析所有依赖
  void resolveDependencies();

private:
  DependencyManager() = default;
  ~DependencyManager() = default;
  DependencyManager(const DependencyManager &) = delete;
  DependencyManager &operator=(const DependencyManager &) = delete;

  std::vector<PendingUpdate> pending_updates_;
};

#endif // _DB_DEPENDENCY_MANAGER_H_
