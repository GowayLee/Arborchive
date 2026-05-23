#ifndef _LAMBDA_PROCESSOR_H_
#define _LAMBDA_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include <clang/AST/ExprCXX.h>
#include <string>
#include <unordered_set>

namespace clang {
class FieldDecl;
class LambdaCapture;
} // namespace clang

class TypeProcessor;
class VariableProcessor;

class Lambda_Processor : public BaseProcessor {
public:
  Lambda_Processor(clang::ASTContext *ast_context,
                   const clang::PrintingPolicy pp,
                   TypeProcessor *type_processor,
                   VariableProcessor *variable_processor)
      : BaseProcessor(ast_context, pp), type_processor_(type_processor),
        variable_processor_(variable_processor) {}
  ~Lambda_Processor() = default;

  void processLambdaExpr(clang::LambdaExpr *expr);

private:
  int getOrCreateLambdaExprId(clang::LambdaExpr *expr);
  std::string getDefaultCaptureString(const clang::LambdaExpr *expr) const;
  void recordCaptures(const clang::LambdaExpr *expr, int lambdaExprId);
  const clang::FieldDecl *
  resolveCaptureField(const clang::LambdaExpr *expr,
                      const clang::LambdaCapture *capture) const;
  int resolveCaptureFieldId(const clang::FieldDecl *field) const;
  int getCaptureLocationId(const clang::LambdaCapture *capture) const;
  bool isCapturedByReference(const clang::LambdaCapture *capture) const;

  TypeProcessor *type_processor_ = nullptr;
  VariableProcessor *variable_processor_ = nullptr;
  std::unordered_set<int> processed_lambda_exprs_;
};

#endif // _LAMBDA_PROCESSOR_H_
