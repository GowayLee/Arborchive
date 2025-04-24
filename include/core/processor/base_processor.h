#ifndef _BASE_PROCESSOR_H_
#define _BASE_PROCESSOR_H_

#include <clang/AST/ASTContext.h>

class BaseProcessor {
protected:
  clang::ASTContext *ast_context_ = nullptr;

public:
  BaseProcessor(clang::ASTContext *ast_context) : ast_context_(ast_context) {};
  ~BaseProcessor() = default;
};

#endif // _BASE_PROCESSOR_H_
