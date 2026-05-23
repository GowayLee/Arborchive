#include "core/processor/lambda_processor.h"
#include "core/processor/type_processor.h"
#include "core/processor/variable_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/expr.h"
#include "model/db/lambda.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/logger/macros.h"
#include <clang/AST/DeclCXX.h>
#include <clang/AST/LambdaCapture.h>
#include <clang/Basic/Lambda.h>
#include <llvm/ADT/DenseMap.h>

void Lambda_Processor::processLambdaExpr(clang::LambdaExpr *expr) {
  if (!expr)
    return;

  int lambdaExprId = getOrCreateLambdaExprId(expr);
  if (lambdaExprId == -1)
    return;

  DbModel::Lambda lambda = {lambdaExprId, getDefaultCaptureString(expr),
                            expr->hasExplicitResultType()};
  STG.insertClassObj(lambda);

  if (processed_lambda_exprs_.insert(lambdaExprId).second)
    recordCaptures(expr, lambdaExprId);
}

int Lambda_Processor::getOrCreateLambdaExprId(clang::LambdaExpr *expr) {
  if (!expr || !ast_context_)
    return -1;

  KeyType exprKey = KeyGen::Expr_::makeKey(expr, ast_context_);
  if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
    return *cachedId;

  LocIdPair *locIdPair = SrcLocRecorder::processExpr(expr, ast_context_);
  int locationId = locIdPair ? locIdPair->spec_id : -1;

  DbModel::Expr exprModel = {GENID(Expr),
                             static_cast<int>(ExprKind::LAMBDAEXPR),
                             locationId};
  INSERT_EXPR_CACHE(exprKey, exprModel.id);
  STG.insertClassObj(exprModel);
  return exprModel.id;
}

std::string
Lambda_Processor::getDefaultCaptureString(const clang::LambdaExpr *expr) const {
  if (!expr)
    return "none";

  switch (expr->getCaptureDefault()) {
  case clang::LCD_None:
    return "none";
  case clang::LCD_ByCopy:
    return "by_copy";
  case clang::LCD_ByRef:
    return "by_reference";
  }

  return "none";
}

void Lambda_Processor::recordCaptures(const clang::LambdaExpr *expr,
                                      int lambdaExprId) {
  if (!expr || lambdaExprId == -1)
    return;

  unsigned captureIndex = 0;
  for (auto captureIt = expr->capture_begin(); captureIt != expr->capture_end();
       ++captureIt, ++captureIndex) {
    const clang::LambdaCapture *capture = captureIt;
    if (!capture)
      continue;

    const clang::FieldDecl *field = resolveCaptureField(expr, capture);
    int fieldId = resolveCaptureFieldId(field);
    if (fieldId == -1) {
      LOG_DEBUG << "Skipping lambda capture without stable member field"
                << std::endl;
      continue;
    }

    int locationId = getCaptureLocationId(capture);
    if (locationId == -1) {
      LOG_DEBUG << "Skipping lambda capture without stable source location"
                << std::endl;
      continue;
    }

    DbModel::LambdaCapture captureRow = {
        GENID(LambdaCapture),
        lambdaExprId,
        static_cast<int>(captureIndex),
        fieldId,
        isCapturedByReference(capture),
        capture->isImplicit(),
        locationId};
    STG.insertClassObj(captureRow);
  }
}

const clang::FieldDecl *Lambda_Processor::resolveCaptureField(
    const clang::LambdaExpr *expr, const clang::LambdaCapture *capture) const {
  if (!expr || !capture || expr->isInitCapture(capture))
    return nullptr;

  const clang::CXXRecordDecl *lambdaClass = expr->getLambdaClass();
  if (!lambdaClass)
    return nullptr;

  llvm::DenseMap<const clang::ValueDecl *, clang::FieldDecl *> captureFields;
  clang::FieldDecl *thisCapture = nullptr;
  lambdaClass->getCaptureFields(captureFields, thisCapture);

  if (capture->capturesVariable()) {
    const clang::ValueDecl *capturedVar = capture->getCapturedVar();
    auto fieldIt = captureFields.find(capturedVar);
    if (fieldIt != captureFields.end())
      return fieldIt->second;
  }

  if (capture->capturesThis())
    return thisCapture;

  return nullptr;
}

int Lambda_Processor::resolveCaptureFieldId(
    const clang::FieldDecl *field) const {
  if (!field || !type_processor_ || !variable_processor_)
    return -1;

  int typeId = type_processor_->processType(field->getType().getTypePtr());
  return variable_processor_->resolveMemberVarId(field, typeId);
}

int Lambda_Processor::getCaptureLocationId(
    const clang::LambdaCapture *capture) const {
  if (!capture || !ast_context_)
    return -1;

  clang::SourceLocation loc = capture->getLocation();
  if (loc.isInvalid())
    return -1;

  LocIdPair *locIdPair = SrcLocRecorder::processDefault(loc, loc, ast_context_);
  return locIdPair ? locIdPair->spec_id : -1;
}

bool Lambda_Processor::isCapturedByReference(
    const clang::LambdaCapture *capture) const {
  if (!capture)
    return false;

  switch (capture->getCaptureKind()) {
  case clang::LCK_ByRef:
  case clang::LCK_This:
    return true;
  case clang::LCK_ByCopy:
  case clang::LCK_StarThis:
  case clang::LCK_VLAType:
    return false;
  }

  return false;
}
