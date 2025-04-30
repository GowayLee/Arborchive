#ifndef _FUNCTION_PROCESSOR_H_
#define _FUNCTION_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "model/db/function.h"
#include <iostream>

class FunctionProcessor : public BaseProcessor {
public:
  void routerProcess(const clang::FunctionDecl *decl) const;
  void processCXXConstructor(const clang::CXXConstructorDecl *decl) const;
  void processCXXDestructor(const clang::CXXDestructorDecl *decl) const;
  void processCXXConversion(const clang::CXXConversionDecl *decl) const;
  void processCXXDeductionGuide(const clang::CXXDeductionGuideDecl *decl) const;
  void processOperatorFunc(const clang::FunctionDecl *decl) const;
  void processBuiltinFunc(const clang::FunctionDecl *decl) const;
  void processUserDefinedLiteral(const clang::FunctionDecl *decl) const;
  void processNormalFunc(const clang::FunctionDecl *decl) const;

  FunctionProcessor(clang::ASTContext *ast_context)
      : BaseProcessor(ast_context) {};
  ~FunctionProcessor() = default;

private:
  int handleBaseFunc(const clang::FunctionDecl *decl,
                     const FuncType type) const;
};

#endif // _FUNCTION_PROCESSOR_H_
