#include "core/processor/expr_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/expr.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/stmt.h"
#include "util/key_generator/variable.h"
#include "util/logger/macros.h"

int ExprProcessor::processBaseExpr(Expr *expr, ExprType exprType) {
  KeyType exprKey = KeyGen::Expr_::makeKey(expr, *ast_context_);
  LocIdPair *locIdPair = SrcLocRecorder::processExpr(expr, ast_context_);

  DbModel::Expr exprModel = {GENID(Expr), static_cast<int>(exprType),
                             locIdPair->spec_id};

  INSERT_STMT_CACHE(exprKey, exprModel.id);
  STG.insertClassObj(exprModel);
  return exprModel.id;
}

void ExprProcessor::processDeclRef(DeclRefExpr *expr) {
  ValueDecl *valueDecl = expr->getDecl();

  if (auto *VarDecl = dyn_cast<clang::VarDecl>(valueDecl))
    recordVarBindExpr(VarDecl, expr);
}

void ExprProcessor::recordVarBindExpr(VarDecl *VD, DeclRefExpr *expr) {
  VarBindType varBindType =
      VarBindType::VARACCESS; // TODO: ctorfieldinit, dtorfielddestruct
  ExprType exprType = ExprType::VARACCESS;
  int exprId = processBaseExpr(expr, exprType);

  DbModel::VarBindExpr varBindExprModel = {GENID(VarBindExpr), exprId,
                                           static_cast<int>(varBindType)};

  // Search VarID from Variable Cache
  // Since Variable should be declared before ref, so ID in the cache is
  // promising
  KeyType varKey = KeyGen::Var::makeKey(VD, VD->getASTContext());
  LOG_DEBUG << "Searching variable cache for " << varKey << std::endl;
  int cachedVarId = -1;
  if (auto cachedId = SEARCH_VARIABLE_CACHE(varKey))
    cachedVarId = *cachedId;
  LOG_DEBUG << "Found variable ID: " << cachedVarId << std::endl;

  DbModel::VarBind varBindModel = {exprId, cachedVarId};

  STG.insertClassObj(varBindExprModel);
  STG.insertClassObj(varBindModel);
}
