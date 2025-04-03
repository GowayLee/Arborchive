#include "db/dependency_manager.h"
#include "model/dependency/base_dep.h"
#include "util/logger/macros.h"
#include <memory>

DependencyManager &DependencyManager::getInstance() {
  static DependencyManager instance;
  return instance;
}

void DependencyManager::addPendingModelDep(std::unique_ptr<BaseDep> dep) {
  std::lock_guard<std::mutex> lock(mutex_);
  pending_models_.push_back(dep);
  LOG_DEBUG << "Added pending model: " << dep->getName()
            << " to dependency manager" << std::endl;
}

void DependencyManager::processPendingModels() {
  std::lock_guard<std::mutex> lock(mutex_);
  while (true) {
    std::vector<std::unique_ptr<BaseDep>> successModels;
    size_t preSize = pending_models_.size();
    for (auto &model : pending_models_) {
      if (model->solve_dependence()) {
        successModels.push_back(model);
        LOG_DEBUG << "Successfully resolved dependencies for model: "
                  << model->getName() << std::endl;
      } else
        LOG_DEBUG << "Failed to resolve dependencies for model: "
                  << model->getName() << std::endl;
    }

    pending_models_.erase(successModels.begin(), successModels.end());
    size_t postSize = pending_models_.size();
    if (postSize == 0) {
      LOG_DEBUG << "Finished processing pending models" << std::endl;
      clear();
      break;
    }
    if (preSize == postSize) {
      LOG_ERROR << "Failed to resolve dependencies for some models"
                << std::endl;
      clear();
      break;
    }
  }
  LOG_ERROR << "Failed to resolve dependencies for all models" << std::endl;
}

void DependencyManager::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  pending_models_.clear();
  LOG_DEBUG << "Cleared all pending models from dependency manager"
            << std::endl;
}
