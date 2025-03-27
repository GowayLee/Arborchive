#ifndef _LOCATION_PROCESSOR_H_
#define _LOCATION_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "model/sql/location_model.h"
#include <clang-c/Index.h>
#include <memory>

class LocationProcessor : public BaseProcessor {
public:
  virtual ~LocationProcessor() = default;
  void handle(CXCursor cursor) override = 0;

protected:
  void processStatement(CXCursor cursor);
  void processExpression(CXCursor cursor);
};

// 导出所有具体的处理器类
class DeclStmtProcessor : public LocationProcessor {
public:
  void handle(CXCursor cursor) override;
};

class CompoundStmtProcessor : public LocationProcessor {
public:
  void handle(CXCursor cursor) override;
};

class DeclRefExprProcessor : public LocationProcessor {
public:
  void handle(CXCursor cursor) override;
};

class CallExprProcessor : public LocationProcessor {
public:
  void handle(CXCursor cursor) override;
};

#endif // _LOCATION_PROCESSOR_H_