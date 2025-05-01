#ifndef _FUNCTION_PROCESSOR_H_
#define _FUNCTION_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "model/db/function.h"
#include <iostream>

using namespace clang;

class FunctionProcessor : public BaseProcessor {
public:
  void routerProcess(const FunctionDecl *decl) const;
  void processCXXConstructor(const CXXConstructorDecl *decl) const;
  void processCXXDestructor(const CXXDestructorDecl *decl) const;
  void processCXXConversion(const CXXConversionDecl *decl) const;
  void processCXXDeductionGuide(const CXXDeductionGuideDecl *decl) const;
  void processOperatorFunc(const FunctionDecl *decl) const;
  void processBuiltinFunc(const FunctionDecl *decl) const;
  void processUserDefinedLiteral(const FunctionDecl *decl) const;
  void processNormalFunc(const FunctionDecl *decl) const;

  FunctionProcessor(ASTContext *ast_context) : BaseProcessor(ast_context) {};
  ~FunctionProcessor() = default;

private:
  int handleBaseFunc(const FunctionDecl *decl, const FuncType type) const;
  void checkBasicInfo(const FunctionDecl *decl, int funcId) const;
};

#endif // _FUNCTION_PROCESSOR_H_
