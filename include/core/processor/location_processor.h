#ifndef _LOCATION_PROCESSOR_H_
#define _LOCATION_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include <clang/Basic/SourceLocation.h>

class LocationProcessor : public BaseProcessor {
public:
  void process(const clang::SourceLocation beginLoc,
               const clang::SourceLocation endLoc);
  LocationProcessor(clang::ASTContext *ast_context)
      : BaseProcessor(ast_context) {};
  ~LocationProcessor() = default;
};

#endif // _LOCATION_PROCESSOR_H_