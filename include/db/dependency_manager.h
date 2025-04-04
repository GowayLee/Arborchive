#ifndef _DEPENDENCY_MANAGER_H_
#define _DEPENDENCY_MANAGER_H_

#include "model/sql/sql_model.h"
#include "model/dependency/base_dep.h"
#include <memory>
#include <mutex>
#include <vector>

class DependencyManager {
public:
  static DependencyManager &getInstance();

  void processPendingModels();

  void addPendingModelDep(std::unique_ptr<BaseDep> &&dep);
  void clear();

private:
  DependencyManager() = default;
  DependencyManager(const DependencyManager &) = delete;
  DependencyManager &operator=(const DependencyManager &) = delete;

  mutable std::mutex mutex_;
  std::vector<std::unique_ptr<BaseDep>> pending_models_;
};

#endif // _DEPENDENCY_MANAGER_H_