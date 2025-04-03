#include "db/dependency_manager.h"
#include "util/logger/macros.h"
#include <memory>

DependencyManager &DependencyManager::getInstance() {
  static DependencyManager instance;
  return instance;
}

void DependencyManager::addPendingModel(std::shared_ptr<SQLModel> model) {
  std::lock_guard<std::mutex> lock(mutex_);
  pending_models_.push_back(model);
  LOG_DEBUG << "Added pending model to dependency manager";
}

void DependencyManager::processPendingModels() {
  for (short i = 0; i <= 3; i++) {
    std::vector<std::shared_ptr<SQLModel>> successModels;
    for (auto &model : pending_models_) {
      if (model->try_solve_dependence()) {
        successModels.push_back(model);
        LOG_DEBUG << "Successfully resolved dependencies for model";
      } else
        LOG_DEBUG << "Failed to resolve dependencies for model";
    }

    pending_models_.erase(successModels.begin(), successModels.end());
    if (pending_models_.empty()) {
      LOG_DEBUG << "Finished processing pending models" << std::endl;
      clear();
      break;
    }
  }
  LOG_ERROR << "Failed to resolve dependencies for all models" << std::endl;
}

void DependencyManager::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  pending_models_.clear();
  LOG_DEBUG << "Cleared all pending models from dependency manager";
}