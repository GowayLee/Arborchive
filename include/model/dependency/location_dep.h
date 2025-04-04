#ifndef _LOCATION_DEP_H_
#define _LOCATION_DEP_H_

#include "db/async_manager.h"
#include "model/dependency/base_dep.h"
#include "model/sql/container_model.h"
#include "model/sql/sql_model.h"
#include "util/logger/macros.h"
#include <memory>

class LocationDep : public BaseDep {
public:
  LocationDep(std::unique_ptr<SQLModel> &&sql_model)
      : BaseDep(std::move(sql_model)) {}

  virtual bool solve_dependence() const override {
    LOG_DEBUG << "Start solve dependence for " << sql_model_->getTableName()
              << std::endl;
    auto &db = AsyncDatabaseManager::getInstance();

    if (dependencies_.empty()) {
      LOG_ERROR << "No dependencies found for " << sql_model_->getTableName()
                << std::endl;
      return false;
    }
    auto &dep = dependencies_[0];
    std::string sql =
        "SELECT * FROM " + dep.table + " WHERE " + dep.field + "=" + dep.value;
    auto results = db.queryModels<FileModel>(sql);
    if (results.empty()) {
      LOG_WARNING << "Solve dependence failed, model: " << getName()
                  << std::endl;
      return false;
    }
    sql_model_->setField("container", results[0]->getField("id"));

    // 更新模型到数据库
    db.pushModel(sql_model_->update_sql());

    return true;
  }
};

#endif