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
  LocationDep(std::shared_ptr<SQLModel> sql_model) : BaseDep(sql_model) {}

  virtual bool solve_dependence() const override {
    auto &db = AsyncDatabaseManager::getInstance();
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

    return true;
  }
};

#endif