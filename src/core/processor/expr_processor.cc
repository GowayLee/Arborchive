#include "core/processor/expr_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/expr.h"
#include "model/db/operator.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/stmt.h"
#include "util/key_generator/variable.h"
#include "util/logger/macros.h"
#include <clang/AST/Expr.h>

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

void ExprProcessor::processUnaryOperator(const UnaryOperator *op) {
  ExprType exprType;
  OpExprType opExprType = OpExprType::UN_OP_EXPR;
  UnOpExprType unOpExprType = UnOpExprType::_UNKNOWN_;                // -1
  UnArithOpExprType unArithOpExprType = UnArithOpExprType::_UNKNOWN_; // -1
  UnBitwiseOpExprType unBitwiseOpExprType;
  UnLogOpExprType unLogOpExprType;

  // Use a manual branch number to handle different cases
  // Directly un_op_expr = 0 (default)
  // Further un_arith_op_expr = 1
  // Further un_bitwise_op_expr = 2
  // Further un_log_op_expr = 3
  int branch = 0;

  switch (op->getOpcode()) {
  // 1.1.1. 自增/自减运算
  case UO_PostInc:
    exprType = ExprType::POSTINCREXPR;
    unOpExprType = UnOpExprType::UN_ARITH_OP_EXPR;
    unArithOpExprType = UnArithOpExprType::CREMENT_EXPR;
    branch = 1;
    break;
  case UO_PostDec:
    exprType = ExprType::POSTDECREXPR;
    unOpExprType = UnOpExprType::UN_ARITH_OP_EXPR;
    unArithOpExprType = UnArithOpExprType::CREMENT_EXPR;
    branch = 1;
    break;
  case UO_PreInc:
    exprType = ExprType::PREINCREXPR;
    unOpExprType = UnOpExprType::UN_ARITH_OP_EXPR;
    unArithOpExprType = UnArithOpExprType::CREMENT_EXPR;
    branch = 1;
    break;
  case UO_PreDec:
    exprType = ExprType::PREDECREXPR;
    unOpExprType = UnOpExprType::UN_ARITH_OP_EXPR;
    unArithOpExprType = UnArithOpExprType::CREMENT_EXPR;
    branch = 1;
    break;
  // 1.1. 一元算术运算
  case UO_Minus:
    exprType = ExprType::ARITHNEGEXPR;
    unOpExprType = UnOpExprType::UN_ARITH_OP_EXPR;
    unArithOpExprType = UnArithOpExprType::ARITHNEGEXPR;
    branch = 1;
    break;
  // 1.2. 一元位运算
  case UO_Not: // ~
    exprType = ExprType::COMPLEMENTEXPR;
    unOpExprType = UnOpExprType::UN_BITEWISE_OP_EXPR;
    unBitwiseOpExprType = UnBitwiseOpExprType::COMPLEMENTEXPR;
    branch = 2;
    break;
  // 1.3. 一元逻辑运算
  case UO_LNot: // !
    exprType = ExprType::NOTEXPR;
    unOpExprType = UnOpExprType::UN_LOG_OP_EXPR;
    unLogOpExprType = UnLogOpExprType::NOTEXPR;
    branch = 3;
    break;
  // 指针与内存相关
  case UO_AddrOf: // &
    exprType = ExprType::ADDRESS_OF;
    unOpExprType = UnOpExprType::ADDRESS_OF;
    branch = 0;
    break;
  case UO_Deref: // *
    exprType = ExprType::INDIRECT;
    unOpExprType = UnOpExprType::INDIRECT;
    branch = 0;
    break;
  default:
    // stand by
    return;
  }

  int exprId = processBaseExpr(const_cast<UnaryOperator *>(op), exprType);

  DbModel::UnOpExpr unOpExprModel;
  DbModel::UnArithOpExpr unArithOpExprModel;
  DbModel::UnBitwiseOpExpr unBitwiseOpExprModel;
  DbModel::UnLogOpExpr unLogOpExprModel;

  switch (branch) {
  case 0:
    unOpExprModel = {GENID(UnOpExpr), exprId, static_cast<int>(unOpExprType)};
    STG.insertClassObj(unOpExprModel);
    break;
  case 1:
    unArithOpExprModel = {GENID(UnArithOpExpr), exprId,
                          static_cast<int>(unArithOpExprType)};
    unOpExprModel = {GENID(UnOpExpr), unArithOpExprModel.id,
                     static_cast<int>(unOpExprType)};
    STG.insertClassObj(unOpExprModel);
    STG.insertClassObj(unArithOpExprModel);
    break;
  case 2:
    unBitwiseOpExprModel = {GENID(UnBitwiseOpExpr), exprId,
                            static_cast<int>(unBitwiseOpExprType)};
    unOpExprModel = {GENID(UnOpExpr), unBitwiseOpExprModel.id,
                     static_cast<int>(unOpExprType)};
    STG.insertClassObj(unOpExprModel);
    STG.insertClassObj(unBitwiseOpExprModel);
    break;
  case 3:
    unLogOpExprModel = {GENID(UnLogOpExpr), exprId,
                        static_cast<int>(unLogOpExprType)};
    unOpExprModel = {GENID(UnOpExpr), unLogOpExprModel.id,
                     static_cast<int>(unOpExprType)};
    STG.insertClassObj(unOpExprModel);
    STG.insertClassObj(unLogOpExprModel);
    break;
  default:
    break;
  }

  // 递归处理子表达式
  // Traverse(op->getSubExpr());
}

void ExprProcessor::processBinaryOperator(const BinaryOperator *op) {
  ExprType expr_type = ExprType::_UNKNOWN_;
  OpExprType opExprType = OpExprType::BIN_OP_EXPR;
  BinOpExprType binOpExprType = BinOpExprType::_UNKNOWN_;
  BinArithOpExprType binArithOpExprType = BinArithOpExprType::_UNKNOWN_;
  POpExprType pOpExprType = POpExprType::_UNKNOWN_;
  BinBitwiseOpExprType binBitwiseOpExprType = BinBitwiseOpExprType::_UNKNOWN_;
  CmpOpExprType cmpOpExprType = CmpOpExprType::_UNKNOWN_;
  EqOpExprType eqOpExprType = EqOpExprType::_UNKNOWN_;
  RelOpExprType relOpExprType = RelOpExprType::_UNKNOWN_;
  BinLogOpExprType binLogOpExprType = BinLogOpExprType::_UNKNOWN_;

  switch (op->getOpcode()) {
  // 2.1. Binary Arithmetic
  case BO_Add:
    if (op->getLHS()->getType()->isPointerType() ||
        op->getRHS()->getType()->isPointerType()) {
      expr_type = ExprType::PADDEXPR;
      binOpExprType = BinOpExprType::BIN_ARITH_OP_EXPR;
      binArithOpExprType = BinArithOpExprType::P_ARITH_OP_EXPR;
      pOpExprType = POpExprType::PADDEXPR;
    } else {
      expr_type = ExprType::ADDEXPR;
      binOpExprType = BinOpExprType::BIN_ARITH_OP_EXPR;
      binArithOpExprType = BinArithOpExprType::ADDEXPR;
    }
    break;
  case BO_Sub:
    if (op->getLHS()->getType()->isPointerType() &&
        op->getRHS()->getType()->isPointerType()) {
      expr_type = ExprType::PDIFFEXPR;
      binOpExprType = BinOpExprType::BIN_ARITH_OP_EXPR;
      binArithOpExprType = BinArithOpExprType::P_ARITH_OP_EXPR;
      pOpExprType = POpExprType::PDIFFEXPR;
    } else if (op->getLHS()->getType()->isPointerType()) {
      expr_type = ExprType::PSUBEXPR;
      binOpExprType = BinOpExprType::BIN_ARITH_OP_EXPR;
      binArithOpExprType = BinArithOpExprType::P_ARITH_OP_EXPR;
      pOpExprType = POpExprType::PSUBEXPR;
    } else {
      expr_type = ExprType::SUBEXPR;
      binOpExprType = BinOpExprType::BIN_ARITH_OP_EXPR;
      binArithOpExprType = BinArithOpExprType::SUBEXPR;
    }
    break;
  case BO_Mul:
    expr_type = ExprType::MULEXPR;
    binOpExprType = BinOpExprType::BIN_ARITH_OP_EXPR;
    binArithOpExprType = BinArithOpExprType::MULEXPR;
    break;
  case BO_Div:
    expr_type = ExprType::DIVEXPR;
    binOpExprType = BinOpExprType::BIN_ARITH_OP_EXPR;
    binArithOpExprType = BinArithOpExprType::DIVEEXPR;
    break;
  case BO_Rem:
    expr_type = ExprType::REMEXPR;
    binOpExprType = BinOpExprType::BIN_ARITH_OP_EXPR;
    binArithOpExprType = BinArithOpExprType::REMEXPR;
    break;
  // 2.2. Binary Bitwise
  case BO_Shl:
    expr_type = ExprType::LSHIFTEXPR;
    binOpExprType = BinOpExprType::BIN_BITWISE_OP_EXPR;
    binBitwiseOpExprType = BinBitwiseOpExprType::LSHIFTEXPR;
    break;
  case BO_Shr:
    expr_type = ExprType::RSHIFTEXPR;
    binOpExprType = BinOpExprType::BIN_BITWISE_OP_EXPR;
    binBitwiseOpExprType = BinBitwiseOpExprType::RSHIFTEXPR;
    break;
  case BO_And:
    expr_type = ExprType::ANDEXPR;
    binOpExprType = BinOpExprType::BIN_BITWISE_OP_EXPR;
    binBitwiseOpExprType = BinBitwiseOpExprType::ANDEXPR;
    break;
  case BO_Or:
    expr_type = ExprType::OREXPR;
    binOpExprType = BinOpExprType::BIN_BITWISE_OP_EXPR;
    binBitwiseOpExprType = BinBitwiseOpExprType::OREXPR;
    break;
  case BO_Xor:
    expr_type = ExprType::XOREXPR;
    binOpExprType = BinOpExprType::BIN_BITWISE_OP_EXPR;
    binBitwiseOpExprType = BinBitwiseOpExprType::XOREXPR;
    break;
  // 2.3. Comparison
  case BO_EQ:
    expr_type = ExprType::EQEXPR;
    binOpExprType = BinOpExprType::CMP_OP_EXPR;
    cmpOpExprType = CmpOpExprType::EQ_OP_EXPR;
    eqOpExprType = EqOpExprType::EQEXPR;
    break;
  case BO_NE:
    expr_type = ExprType::NEEXPR;
    binOpExprType = BinOpExprType::CMP_OP_EXPR;
    cmpOpExprType = CmpOpExprType::EQ_OP_EXPR;
    eqOpExprType = EqOpExprType::NEEXPR;
    break;
  case BO_GT:
    expr_type = ExprType::GTEXPR;
    binOpExprType = BinOpExprType::CMP_OP_EXPR;
    cmpOpExprType = CmpOpExprType::REL_OP_EXPR;
    relOpExprType = RelOpExprType::GTEXPR;
    break;
  case BO_LT:
    expr_type = ExprType::LTEXPR;
    binOpExprType = BinOpExprType::CMP_OP_EXPR;
    cmpOpExprType = CmpOpExprType::REL_OP_EXPR;
    relOpExprType = RelOpExprType::LTEXPR;
    break;
  case BO_GE:
    expr_type = ExprType::GEEXPR;
    binOpExprType = BinOpExprType::CMP_OP_EXPR;
    cmpOpExprType = CmpOpExprType::REL_OP_EXPR;
    relOpExprType = RelOpExprType::GEEXPR;
    break;
  case BO_LE:
    expr_type = ExprType::LEEXPR;
    binOpExprType = BinOpExprType::CMP_OP_EXPR;
    cmpOpExprType = CmpOpExprType::REL_OP_EXPR;
    relOpExprType = RelOpExprType::LEEXPR;
    break;
  // 2.4. 二元逻辑运算
  case BO_LAnd:
    expr_type = ExprType::ANDLOGICALEXPR;
    binOpExprType = BinOpExprType::BIN_LOG_OP_EXPR;
    binLogOpExprType = BinLogOpExprType::ANDLOGICALEXPR;
    break;
  case BO_LOr:
    expr_type = ExprType::ORLOGICALEXPR;
    binOpExprType = BinOpExprType::BIN_LOG_OP_EXPR;
    binLogOpExprType = BinLogOpExprType::ORLOGICALEXPR;
    break;
  // 3. 赋值运算符
  case BO_Assign:
    expr_type = ExprType::ASSIGNEXPR;
    break;
  case BO_AddAssign:
    expr_type = ExprType::ASSIGNADDEXPR;
    break;
  case BO_SubAssign:
    expr_type = ExprType::ASSIGNSUBEXPR;
    break;
  case BO_MulAssign:
    expr_type = ExprType::ASSIGNMULEXPR;
    break;
  case BO_DivAssign:
    expr_type = ExprType::ASSIGNDIVEXPR;
    break;
  case BO_RemAssign:
    expr_type = ExprType::ASSIGNREMEXPR;
    break;
  case BO_ShlAssign:
    expr_type = ExprType::ASSIGNLSHIFTEXPR;
    break;
  case BO_ShrAssign:
    expr_type = ExprType::ASSIGNRSHIFTEXPR;
    break;
  case BO_AndAssign:
    expr_type = ExprType::ASSIGNANDEXPR;
    break;
  case BO_OrAssign:
    expr_type = ExprType::ASSIGNOREXPR;
    break;
  case BO_XorAssign:
    expr_type = ExprType::ASSIGNXOREXPR;
    break;
  default:
    return;
  }

  processBaseExpr(const_cast<BinaryOperator *>(op), expr_type);

  // Traverse(op->getLHS());
  // Traverse(op->getRHS());
}

void ExprProcessor::processConditionalOperator(const ConditionalOperator *op) {
  processBaseExpr(const_cast<ConditionalOperator *>(op),
                  ExprType::CONDITIONALEXPR);
  // Traverse(op->getCond());
  // Traverse(op->getTrueExpr());
  // Traverse(op->getFalseExpr());
}
