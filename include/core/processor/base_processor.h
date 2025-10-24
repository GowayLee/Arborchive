#ifndef _BASE_PROCESSOR_H_
#define _BASE_PROCESSOR_H_

#include <clang/AST/ASTContext.h>

class BaseProcessor {
protected:
  clang::ASTContext *ast_context_;
  clang::PrintingPolicy pp_;

public:
  BaseProcessor(clang::ASTContext *ast_context, const clang::PrintingPolicy pp)
      : ast_context_(ast_context), pp_(pp) {
    pp_.SuppressTagKeyword = true;
    pp_.SuppressScope = false;
  };
  ~BaseProcessor() = default;
};

#endif // _BASE_PROCESSOR_H_
