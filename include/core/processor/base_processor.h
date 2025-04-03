#ifndef _BASE_PROCESSOR_H_
#define _BASE_PROCESSOR_H_

#include "db/dependency_manager.h"
#include <clang-c/Index.h>
#include <functional>
#include <map>
#include <memory>

class BaseProcessor {
protected:
  DependencyManager &dep_manager_ = DependencyManager::getInstance();

public:
  using FactoryFunc = std::function<std::unique_ptr<BaseProcessor>()>;

  virtual void handle(CXCursor cursor) = 0;

  // 获取注册表实例
  static std::map<CXCursorKind, FactoryFunc> &registry() {
    static std::map<CXCursorKind, FactoryFunc> instance;
    return instance;
  }

  // 注册处理器工厂函数
  static void registerProcessor(CXCursorKind kind, FactoryFunc factory) {
    registry().emplace(kind, std::move(factory));
  }
};

#endif // _BASE_PROCESSOR_H_
