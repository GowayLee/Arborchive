#ifndef _FUNCTION_PROCESSOR_H_
#define _FUNCTION_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/function.h"

using namespace clang;

class FunctionProcessor : public BaseProcessor {
public:
  void routerProcess(const FunctionDecl *decl);
  void processCXXConstructor(const CXXConstructorDecl *decl);
  void processCXXDestructor(const CXXDestructorDecl *decl);
  void processCXXConversion(const CXXConversionDecl *decl);
  void processCXXDeductionGuide(const CXXDeductionGuideDecl *decl);
  void processOperatorFunc(const FunctionDecl *decl);
  void processBuiltinFunc(const FunctionDecl *decl);
  void processUserDefinedLiteral(const FunctionDecl *decl);
  void processNormalFunc(const FunctionDecl *decl);

  FunctionProcessor(ASTContext *ast_context) : BaseProcessor(ast_context) {};
  ~FunctionProcessor() = default;

private:
  // Cache for common information
  // Since the RecursiveVisitor will firstly visit FunctionDecl node, than visit
  // related subclasses of FunctionDecl, so, we can cache the information here
  LocIdPair *_locIdPair;
  int _funcId;
  int _funcDeclId;
  int _typeId;

  void handleBaseFunc(const FunctionDecl *decl, const FuncType type);
  void recordBasicInfo(const FunctionDecl *decl) const;
  void recordEntryPoint(const FunctionDecl *decl) const;
  void recordException(const clang::FunctionDecl *decl) const;
  void recordTypedef(const FunctionDecl *decl) const;
  void recordReturnType(const FunctionDecl *decl);
  void recordCoroutine(const FunctionDecl *decl);
};

#endif // _FUNCTION_PROCESSOR_H_
