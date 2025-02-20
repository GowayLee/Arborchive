#ifndef _BASE_PROCESSOR_H_
#define _BASE_PROCESSOR_H_

#define REGISTER_PROCESSOR(ProcessorClass, CursorKind)                         \
  static AutoRegister<ProcessorClass> ProcessorClass##Reg(CursorKind)

#include <clang-c/Index.h>
#include <functional>
#include <map>
#include <memory>

class BaseProcessor {
public:
  virtual void handle(CXCursor cursor) = 0; // 处理方法，接收AST节点
  static std::map<CXCursorKind, std::function<std::unique_ptr<BaseProcessor>()>>
      registry; // 注册表
};

template <typename T> class AutoRegister {
public:
  AutoRegister(CXCursorKind kind) {
    BaseProcessor::registry[kind] = []() { return std::make_unique<T>(); };
  }
};

#endif // _BASE_PROCESSOR_H_
