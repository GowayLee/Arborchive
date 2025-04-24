#include "db/storage_facade.h"
#include "db/storage.h"

void StorageFacade::initOrm(const DatabaseConfig config) {
  Storage::getInstance().initialize(config);
}

template <typename T> void StorageFacade::insertClassObj(T &&obj) {
  auto storage = Storage::getInstance().getStorage();
  auto statement = storage->prepare(replace(std::forward<T>(obj)));
  storage->execute(statement);
}

void StorageFacade::transaction(const std::function<bool()> &f) {
  auto storage = Storage::getInstance().getStorage();
  storage->transaction(f);
}

#include "storage_facade_instantiations.inc"
// // 显式实例化所有被调用的模板特化
// // clang-format off
// template void StorageFacade::insertClassObj<DbModel::Compilation&>(DbModel::Compilation&);
// template void StorageFacade::insertClassObj<DbModel::CompilationArg&>(DbModel::CompilationArg&);
// template void StorageFacade::insertClassObj<DbModel::CompilationTime&>(DbModel::CompilationTime&);
// template void StorageFacade::insertClassObj<DbModel::File&>(DbModel::File&);
// template void StorageFacade::insertClassObj<DbModel::Container&>(DbModel::Container&);
// template void StorageFacade::insertClassObj<DbModel::CompilationFinished&>(DbModel::CompilationFinished&);
// template void StorageFacade::insertClassObj<DbModel::LocationStmt&>(DbModel::LocationStmt&);
// template void StorageFacade::insertClassObj<DbModel::Location&>(DbModel::Location&);
// // clang-format on