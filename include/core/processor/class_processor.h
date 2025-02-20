#ifndef _CLASS_PROCESSOR_H_
#define _CLASS_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "util/logger/macros.h"
#include <clang-c/Index.h>
#include <iostream>

class ClassDeclProcessor : public BaseProcessor {
public:
  void handle(CXCursor cursor) override {
    std::string functionName =
        clang_getCString(clang_getCursorSpelling(cursor));
    LOG_DEBUG << "Handling Class Declaration: " << functionName << std::endl;
  }
};

// 注册处理器
REGISTER_PROCESSOR(ClassDeclProcessor, CXCursor_ClassDecl);

#endif // _CLASS_PROCESSOR_H_
