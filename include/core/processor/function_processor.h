#ifndef _FUNCTION_PROCESSOR_H_
#define _FUNCTION_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/function.h"

using namespace clang;

class FunctionProcessor : public BaseProcessor {
public:
  int routerProcess(const FunctionDecl *decl);
  int processCXXConstructor(const CXXConstructorDecl *decl);
  int processCXXDestructor(const CXXDestructorDecl *decl);
  int processCXXConversion(const CXXConversionDecl *decl);
  int processCXXDeductionGuide(const CXXDeductionGuideDecl *decl);
  int processOperatorFunc(const FunctionDecl *decl);
  int processBuiltinFunc(const FunctionDecl *decl);
  int processUserDefinedLiteral(const FunctionDecl *decl);
  int processNormalFunc(const FunctionDecl *decl);

  FunctionProcessor(ASTContext *ast_context, const PrintingPolicy pp)
      : BaseProcessor(ast_context, pp) {};
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
