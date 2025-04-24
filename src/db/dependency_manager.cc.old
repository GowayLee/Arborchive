#include "db/dependency_manager.h"
#include "model/dependency/base_dep.h"
#include "util/logger/macros.h"
#include <memory>

DependencyManager &DependencyManager::getInstance() {
  static DependencyManager instance;
  return instance;
}

void DependencyManager::addPendingModelDep(std::unique_ptr<BaseDep> &&dep) {
  std::lock_guard<std::mutex> lock(mutex_);
  LOG_DEBUG << "Added pending model: " << dep->getName() << std::endl;
  pending_models_.push_back(std::move(dep));
}

void DependencyManager::processPendingModels() {
  std::lock_guard<std::mutex> lock(mutex_);
  LOG_DEBUG << "Processing pending models" << std::endl;
  while (true) {
    std::vector<std::unique_ptr<BaseDep>> failModels;
    for (auto &model : pending_models_) {
      if (model->solve_dependence()) {
        LOG_DEBUG << "Successfully resolved dependencies for model: "
                  << model->getName() << std::endl;
      } else {
        LOG_DEBUG << "Failed to resolve dependencies for model: "
                  << model->getName() << std::endl;
        failModels.push_back(std::move(model));
      }
    }

    LOG_DEBUG << "Now pending models: " << pending_models_.size() << std::endl;
    if (failModels.size() == pending_models_.size()) {
      LOG_ERROR << "Failed to resolve dependencies for some models, try to "
                   "solve in the next turn."
                << std::endl;
      clear();
      return;
    }
    if (failModels.size() == 0) {
      LOG_DEBUG << "Finished processing pending models" << std::endl;
      clear();
      return;
    }
    pending_models_ = std::move(failModels);
  }
  LOG_ERROR << "Failed to resolve dependencies for all models" << std::endl;
}

void DependencyManager::clear() {
  pending_models_.clear();
  LOG_DEBUG << "Cleared all pending models from dependency manager"
            << std::endl;
}
