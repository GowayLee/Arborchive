#ifndef _LOCATION_PROCESSOR_H_
#define _LOCATION_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "model/db/location.h"
#include <clang/Basic/SourceLocation.h>

class LocationProcessor : public BaseProcessor {
public:
  int locModelId;

  void processDefault(const clang::SourceLocation beginLoc,
                      const clang::SourceLocation endLoc);
  void processStmt(const clang::SourceLocation beginLoc,
                   const clang::SourceLocation endLoc);
  void processExpr(const clang::SourceLocation beginLoc,
                   const clang::SourceLocation endLoc);
  void process(const clang::SourceLocation beginLoc,
               const clang::SourceLocation endLoc, const LocationType type);
  LocationProcessor(clang::ASTContext *ast_context)
      : BaseProcessor(ast_context) {};
  ~LocationProcessor() = default;
};

#endif // _LOCATION_PROCESSOR_H_