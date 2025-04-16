#ifndef _CLASS_PROCESSOR_H_
#define _CLASS_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "util/logger/macros.h"
#include <clang-c/Index.h>
#include <iostream>

class ClassDeclProcessor : public BaseProcessor {
public:
  void process(const clang::SourceLocation beginLoc,
               const clang::SourceLocation endLoc);
  ClassDeclProcessor(clang::ASTContext *ast_context)
      : BaseProcessor(ast_context) {};
  ~ClassDeclProcessor() = default;
};

#endif // _CLASS_PROCESSOR_H_
