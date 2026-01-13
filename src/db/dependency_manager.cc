#include "db/dependency_manager.h"
#include "db/cache_repository.h"
#include "model/db/element.h"
#include "model/db/expr.h"
#include "model/db/function.h"
#include "model/db/stmt.h"
#include "model/db/type.h"
#include "model/db/variable.h"
#include "util/logger/macros.h"
#include <optional>

DependencyManager &DependencyManager::instance() {
  static DependencyManager instance;
  return instance;
}

void DependencyManager::addDependency(const PendingUpdate &update) {
  pending_updates_.push_back(update);
}

void DependencyManager::resolveDependencies() {
  for (const auto &update : pending_updates_) {
    std::optional<int> resolvedId;

    // 根据 keyType 从正确的缓存中查找 ID
    switch (update.keyType) {
    case CacheType::FUNCTION:
      resolvedId = CacheManager::instance()
                       .getRepository<CacheRepository<DbModel::Function>>()
                       .find(update.dependencyKey);
      break;
    case CacheType::TYPE:
      resolvedId = CacheManager::instance()
                       .getRepository<CacheRepository<DbModel::Type>>()
                       .find(update.dependencyKey);
      break;
    case CacheType::USERTYPE:
      resolvedId = CacheManager::instance()
                       .getRepository<CacheRepository<DbModel::UserType>>()
                       .find(update.dependencyKey);
      break;
    case CacheType::STMT:
      resolvedId = CacheManager::instance()
                       .getRepository<CacheRepository<DbModel::Stmt>>()
                       .find(update.dependencyKey);
      break;
    case CacheType::EXPR:
      resolvedId = CacheManager::instance()
                       .getRepository<CacheRepository<DbModel::Expr>>()
                       .find(update.dependencyKey);
      break;
    case CacheType::VARIABLE:
      resolvedId = CacheManager::instance()
                       .getRepository<CacheRepository<DbModel::Variable>>()
                       .find(update.dependencyKey);
      break;
    case CacheType::ELEMENT:
      resolvedId =
          CacheManager::instance()
              .getRepository<CacheRepository<DbModel::ParameterizedElement>>()
              .find(update.dependencyKey);
      break;
    case CacheType::MEMBERVERY:
      resolvedId = CacheManager::instance()
                       .getRepository<CacheRepository<DbModel::MemberVar>>()
                       .find(update.dependencyKey);
      break;
    }

    if (resolvedId) {
      // 找到ID，执行之前注册的回调函数
      update.updater(*resolvedId);
      LOG_DEBUG << "Dependency resolved for key: " << update.dependencyKey
                << std::endl;
    } else {
      LOG_WARNING << "Failed to resolve dependency for key: "
                  << update.dependencyKey << std::endl;
    }
  }
  // 清空已处理的依赖，为下一轮分析做准备
  pending_updates_.clear();
}
