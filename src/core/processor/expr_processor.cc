#include "core/processor/expr_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/expr.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/stmt.h"
#include "util/key_generator/values.h"
#include "util/key_generator/variable.h"
#include "util/logger/macros.h"
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>

int ExprProcessor::processBaseExpr(Expr *expr, ExprKind exprKind) {
  KeyType exprKey = KeyGen::Expr_::makeKey(expr, *ast_context_);
  LocIdPair *locIdPair = SrcLocRecorder::processExpr(expr, ast_context_);

  DbModel::Expr exprModel = {GENID(Expr), static_cast<int>(exprKind),
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
  ExprKind exprKind = ExprKind::VARACCESS;
  int exprId = processBaseExpr(expr, exprKind);

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

  STG.insertClassObj(varBindModel);
}

void ExprProcessor::processUnaryOperator(const UnaryOperator *op) {
  ExprKind exprType;
  switch (op->getOpcode()) {
  // 1.1.1. 自增/自减运算
  case UO_PostInc:
    exprType = ExprKind::POSTINCREXPR;
    break;
  case UO_PostDec:
    exprType = ExprKind::POSTDECREXPR;
    break;
  case UO_PreInc:
    exprType = ExprKind::PREINCREXPR;
    break;
  case UO_PreDec:
    exprType = ExprKind::PREDECREXPR;
    break;
  // 1.1. 一元算术运算
  case UO_Minus:
    exprType = ExprKind::ARITHNEGEXPR;
    break;
  // 1.2. 一元位运算
  case UO_Not: // ~
    exprType = ExprKind::COMPLEMENTEXPR;
    break;
  // 1.3. 一元逻辑运算
  case UO_LNot: // !
    exprType = ExprKind::NOTEXPR;
    break;
  // 指针与内存相关
  case UO_AddrOf: // &
    exprType = ExprKind::ADDRESS_OF;
    break;
  case UO_Deref: // *
    exprType = ExprKind::INDIRECT;
    break;
  default:
    // stand by
    return;
  }

  int exprId = processBaseExpr(const_cast<UnaryOperator *>(op), exprType);

  // 递归处理子表达式
  // Traverse(op->getSubExpr());
}

void ExprProcessor::processBinaryOperator(const BinaryOperator *op) {
  ExprKind expr_type = ExprKind::_UNKNOWN_;

  switch (op->getOpcode()) {
  // 2.1. Binary Arithmetic
  case BO_Add:
    if (op->getLHS()->getType()->isPointerType() ||
        op->getRHS()->getType()->isPointerType())
      expr_type = ExprKind::PADDEXPR;
    else
      expr_type = ExprKind::ADDEXPR;
    break;
  case BO_Sub:
    if (op->getLHS()->getType()->isPointerType() &&
        op->getRHS()->getType()->isPointerType())
      expr_type = ExprKind::PDIFFEXPR;
    else if (op->getLHS()->getType()->isPointerType())
      expr_type = ExprKind::PSUBEXPR;
    else
      expr_type = ExprKind::SUBEXPR;
    break;
  case BO_Mul:
    expr_type = ExprKind::MULEXPR;
    break;
  case BO_Div:
    expr_type = ExprKind::DIVEXPR;
    break;
  case BO_Rem:
    expr_type = ExprKind::REMEXPR;
    break;
  // 2.2. Binary Bitwise
  case BO_Shl:
    expr_type = ExprKind::LSHIFTEXPR;
    break;
  case BO_Shr:
    expr_type = ExprKind::RSHIFTEXPR;
    break;
  case BO_And:
    expr_type = ExprKind::ANDEXPR;
    break;
  case BO_Or:
    expr_type = ExprKind::OREXPR;
    break;
  case BO_Xor:
    expr_type = ExprKind::XOREXPR;
    break;
  // 2.3. Comparison
  case BO_EQ:
    expr_type = ExprKind::EQEXPR;
    break;
  case BO_NE:
    expr_type = ExprKind::NEEXPR;
    break;
  case BO_GT:
    expr_type = ExprKind::GTEXPR;
    break;
  case BO_LT:
    expr_type = ExprKind::LTEXPR;
    break;
  case BO_GE:
    expr_type = ExprKind::GEEXPR;
    break;
  case BO_LE:
    expr_type = ExprKind::LEEXPR;
    break;
  // 2.4. 二元逻辑运算
  case BO_LAnd:
    expr_type = ExprKind::ANDLOGICALEXPR;
    break;
  case BO_LOr:
    expr_type = ExprKind::ORLOGICALEXPR;
    break;
  // 3. 赋值运算符
  case BO_Assign:
  case BO_AddAssign:
  case BO_SubAssign:
  case BO_MulAssign:
  case BO_DivAssign:
  case BO_RemAssign:
  case BO_ShlAssign:
  case BO_ShrAssign:
  case BO_AndAssign:
  case BO_OrAssign:
  case BO_XorAssign:
    processAssignExpr(op);
    return;
  default:
    return;
  }

  processBaseExpr(const_cast<BinaryOperator *>(op), expr_type);

  // Traverse(op->getLHS());
  // Traverse(op->getRHS());
}

void ExprProcessor::processConditionalOperator(const ConditionalOperator *op) {
  processBaseExpr(const_cast<ConditionalOperator *>(op),
                  ExprKind::CONDITIONALEXPR);
  // Traverse(op->getCond());
  // Traverse(op->getTrueExpr());
  // Traverse(op->getFalseExpr());
}

void ExprProcessor::processAssignArithExpr(const BinaryOperator *op) {
  ExprKind expr_type = ExprKind::_UNKNOWN_;

  switch (op->getOpcode()) {
  case BO_AddAssign:
    expr_type = ExprKind::ASSIGNADDEXPR;
    break;
  case BO_SubAssign:
    expr_type = ExprKind::ASSIGNSUBEXPR;
    break;
  case BO_MulAssign:
    expr_type = ExprKind::ASSIGNMULEXPR;
    break;
  case BO_DivAssign:
    expr_type = ExprKind::ASSIGNDIVEXPR;
    break;
  case BO_RemAssign:
    expr_type = ExprKind::ASSIGNREMEXPR;
    break;
  default:
    return;
  }

  processBaseExpr(const_cast<BinaryOperator *>(op), expr_type);
}

void ExprProcessor::processAssignBitwiseExpr(const BinaryOperator *op) {
  ExprKind expr_type = ExprKind::_UNKNOWN_;

  switch (op->getOpcode()) {
  case BO_AndAssign:
    expr_type = ExprKind::ASSIGNANDEXPR;
    break;
  case BO_OrAssign:
    expr_type = ExprKind::ASSIGNOREXPR;
    break;
  case BO_XorAssign:
    expr_type = ExprKind::ASSIGNXOREXPR;
    break;
  case BO_ShlAssign:
    expr_type = ExprKind::ASSIGNLSHIFTEXPR;
    break;
  case BO_ShrAssign:
    expr_type = ExprKind::ASSIGNRSHIFTEXPR;
    break;
  default:
    return;
  }

  processBaseExpr(const_cast<BinaryOperator *>(op), expr_type);
}

void ExprProcessor::processAssignPointerExpr(const BinaryOperator *op) {
  ExprKind expr_type = ExprKind::_UNKNOWN_;

  switch (op->getOpcode()) {
  case BO_AddAssign:
    if (op->getLHS()->getType()->isPointerType())
      expr_type = ExprKind::ASSIGNPADDEXPR;
    else
      return;
    break;
  case BO_SubAssign:
    if (op->getLHS()->getType()->isPointerType())
      expr_type = ExprKind::ASSIGNPSUBEXPR;
    else
      return;
    break;
  default:
    return;
  }

  processBaseExpr(const_cast<BinaryOperator *>(op), expr_type);
}

void ExprProcessor::processAssignOpExpr(const BinaryOperator *op) {
  switch (op->getOpcode()) {
  case BO_AddAssign:
  case BO_SubAssign:
  case BO_MulAssign:
  case BO_DivAssign:
  case BO_RemAssign:
    if (op->getLHS()->getType()->isPointerType()) {
      processAssignPointerExpr(op);
    } else {
      processAssignArithExpr(op);
    }
    break;
  case BO_AndAssign:
  case BO_OrAssign:
  case BO_XorAssign:
  case BO_ShlAssign:
  case BO_ShrAssign:
    processAssignBitwiseExpr(op);
    break;
  default:
    return;
  }
}

void ExprProcessor::processAssignExpr(const BinaryOperator *op) {
  switch (op->getOpcode()) {
  case BO_Assign:
    processBaseExpr(const_cast<BinaryOperator *>(op), ExprKind::ASSIGNEXPR);
    break;
  case BO_AddAssign:
  case BO_SubAssign:
  case BO_MulAssign:
  case BO_DivAssign:
  case BO_RemAssign:
  case BO_AndAssign:
  case BO_OrAssign:
  case BO_XorAssign:
  case BO_ShlAssign:
  case BO_ShrAssign:
    processAssignOpExpr(op);
    break;
  default:
    return;
  }
}

void ExprProcessor::processStringLiteral(const StringLiteral *literal) {
  int exprId =
      processBaseExpr(const_cast<StringLiteral *>(literal), ExprKind::LITERAL);

  std::string value = literal->getString().str();
  std::string text = "\"" + value + "\"";

  processLiteralValue(value, text, exprId);
}

void ExprProcessor::processIntegerLiteral(const IntegerLiteral *literal) {
  int exprId =
      processBaseExpr(const_cast<IntegerLiteral *>(literal), ExprKind::LITERAL);

  std::string value = std::to_string(literal->getValue().getSExtValue());
  std::string text = value;

  processLiteralValue(value, text, exprId);
}

void ExprProcessor::processFloatingLiteral(const FloatingLiteral *literal) {
  int exprId = processBaseExpr(const_cast<FloatingLiteral *>(literal),
                               ExprKind::LITERAL);

  llvm::SmallVector<char, 16> buffer;
  literal->getValue().toString(buffer);
  std::string value(buffer.begin(), buffer.end());
  std::string text = value;

  processLiteralValue(value, text, exprId);
}

void ExprProcessor::processCharacterLiteral(const CharacterLiteral *literal) {
  int exprId = processBaseExpr(const_cast<CharacterLiteral *>(literal),
                               ExprKind::LITERAL);

  std::string value = std::to_string(literal->getValue());
  std::string text =
      "'" + std::string(1, static_cast<char>(literal->getValue())) + "'";

  processLiteralValue(value, text, exprId);
}

void ExprProcessor::processBoolLiteral(const CXXBoolLiteralExpr *literal) {
  int exprId = processBaseExpr(const_cast<CXXBoolLiteralExpr *>(literal),
                               ExprKind::LITERAL);

  std::string value = literal->getValue() ? "1" : "0";
  std::string text = literal->getValue() ? "true" : "false";

  processLiteralValue(value, text, exprId);
}

int ExprProcessor::processLiteralValue(const std::string &value,
                                       const std::string &text, int exprId) {
  // Create Values entry
  KeyType valueKey = KeyGen::Values::makeKey(value);
  int valueId = GENID(Values);

  DbModel::Values valuesModel = {valueId, value};
  INSERT_VALUES_CACHE(valueKey, valueId);
  STG.insertClassObj(valuesModel);

  // Create ValueText entry
  KeyType textKey = KeyGen::ValueText::makeKey(text);
  int textId = GENID(ValueText);

  DbModel::ValueText valueTextModel = {textId, text};
  INSERT_VALUETEXT_CACHE(textKey, textId);
  STG.insertClassObj(valueTextModel);

  // Create ValueBind to link expression with value
  recordValueBindExpr(valueId, exprId);

  return valueId;
}

void ExprProcessor::recordValueBindExpr(int valueId, int exprId) {
  DbModel::ValueBind valueBindModel = {valueId, exprId};
  STG.insertClassObj(valueBindModel);
}
