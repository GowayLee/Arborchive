#ifndef _FUNCTION_PROCESSOR_H_
#define _FUNCTION_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "model/db/function.h"
#include <iostream>

class FunctionProcessor : public BaseProcessor {
public:
  int handleFunc(clang::FunctionDecl *decl, FuncType type);
  void processCXXConstructor(clang::CXXConstructorDecl *decl);
  void processCXXDestructor(clang::CXXDestructorDecl *decl);
  void processCXXConversion(clang::CXXConversionDecl *decl);
  void processCXXDeductionGuide(clang::CXXDeductionGuideDecl *decl);

  FunctionProcessor(clang::ASTContext *ast_context)
      : BaseProcessor(ast_context) {};
  ~FunctionProcessor() = default;
};

#endif // _FUNCTION_PROCESSOR_H_
