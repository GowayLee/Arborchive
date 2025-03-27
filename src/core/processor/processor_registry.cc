#include "core/processor/base_processor.h"
#include "core/processor/class_processor.h"
#include "core/processor/location_processor.h"
#include <clang-c/Index.h>

class ProcessorRegistrar {
public:
  ProcessorRegistrar() {
    // 注册ClassDeclProcessor
    BaseProcessor::registerProcessor(CXCursor_ClassDecl, [] {
      return std::make_unique<ClassDeclProcessor>();
    });

    // 注册Location相关处理器
    BaseProcessor::registerProcessor(CXCursor_DeclStmt, [] {
      return std::make_unique<DeclStmtProcessor>();
    });
    BaseProcessor::registerProcessor(CXCursor_CompoundStmt, [] {
      return std::make_unique<CompoundStmtProcessor>();
    });
    BaseProcessor::registerProcessor(CXCursor_DeclRefExpr, [] {
      return std::make_unique<DeclRefExprProcessor>();
    });
    BaseProcessor::registerProcessor(CXCursor_CallExpr, [] {
      return std::make_unique<CallExprProcessor>();
    });
  }
};

// 全局静态注册器实例
ProcessorRegistrar registrar;