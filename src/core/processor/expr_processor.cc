#include "core/processor/expr_processor.h"
#include "core/processor/function_processor.h"
#include "core/processor/specifier_processor.h"
#include "core/processor/type_processor.h"
#include "core/srcloc_recorder.h"
#include "db/dependency_manager.h"
#include "db/storage_facade.h"
#include "model/db/expr.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/function.h"
#include "util/key_generator/values.h"
#include "util/key_generator/variable.h"
#include "util/logger/macros.h"
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprConcepts.h>
#include <clang/AST/ExprCXX.h>
#include <memory>

int ExprProcessor::findCachedExprId(const Expr *expr) const {
  if (!expr || !ast_context_)
    return -1;

  KeyType exprKey = KeyGen::Expr_::makeKey(expr, ast_context_);
  if (exprKey.empty())
    return -1;

  return SEARCH_EXPR_CACHE(exprKey).value_or(-1);
}

bool ExprProcessor::canProcessExprForReference(const Expr *expr) const {
  if (!expr || !ast_context_)
    return false;

  if (expr->getBeginLoc().isInvalid() || expr->getEndLoc().isInvalid())
    return false;

  if (expr->isTypeDependent() || expr->isValueDependent() ||
      expr->isInstantiationDependent() || expr->containsUnexpandedParameterPack())
    return false;

  return llvm::isa<DeclRefExpr, UnaryOperator, BinaryOperator,
                   ConditionalOperator, StringLiteral, IntegerLiteral,
                   FloatingLiteral, CharacterLiteral, CXXBoolLiteralExpr,
                   CXXNullPtrLiteralExpr,
                   CallExpr, CastExpr, ParenExpr, CXXThisExpr,
                   CXXNewExpr, CXXDeleteExpr,
                   ArraySubscriptExpr, InitListExpr,
                   UnaryExprOrTypeTraitExpr>(expr);
}

int ExprProcessor::getOrProcessExprId(const clang::Expr *expr) {
  if (!canProcessExprForReference(expr))
    return -1;

  if (int cachedId = findCachedExprId(expr); cachedId != -1)
    return cachedId;

  if (const auto *declRef = llvm::dyn_cast<DeclRefExpr>(expr)) {
    processDeclRef(const_cast<DeclRefExpr *>(declRef));
  } else if (const auto *thisExpr = llvm::dyn_cast<CXXThisExpr>(expr)) {
    processThisExpr(thisExpr);
  } else if (const auto *unary = llvm::dyn_cast<UnaryOperator>(expr)) {
    processUnaryOperator(unary);
  } else if (const auto *binary = llvm::dyn_cast<BinaryOperator>(expr)) {
    processBinaryOperator(binary);
  } else if (const auto *conditional =
                 llvm::dyn_cast<ConditionalOperator>(expr)) {
    processConditionalOperator(conditional);
  } else if (const auto *stringLiteral =
                 llvm::dyn_cast<StringLiteral>(expr)) {
    processStringLiteral(stringLiteral);
  } else if (const auto *integerLiteral =
                 llvm::dyn_cast<IntegerLiteral>(expr)) {
    processIntegerLiteral(integerLiteral);
  } else if (const auto *floatingLiteral =
                 llvm::dyn_cast<FloatingLiteral>(expr)) {
    processFloatingLiteral(floatingLiteral);
  } else if (const auto *characterLiteral =
                 llvm::dyn_cast<CharacterLiteral>(expr)) {
    processCharacterLiteral(characterLiteral);
  } else if (const auto *boolLiteral =
                 llvm::dyn_cast<CXXBoolLiteralExpr>(expr)) {
    processBoolLiteral(boolLiteral);
  } else if (const auto *nullLiteral =
                 llvm::dyn_cast<CXXNullPtrLiteralExpr>(expr)) {
    processBaseExpr(const_cast<CXXNullPtrLiteralExpr *>(nullLiteral),
                    ExprKind::LITERAL);
  } else if (const auto *callExpr = llvm::dyn_cast<CallExpr>(expr)) {
    processCallExpr(callExpr);
  } else if (const auto *newExpr = llvm::dyn_cast<CXXNewExpr>(expr)) {
    processCXXNewExpr(newExpr);
  } else if (const auto *deleteExpr = llvm::dyn_cast<CXXDeleteExpr>(expr)) {
    processCXXDeleteExpr(deleteExpr);
  } else if (const auto *castExpr = llvm::dyn_cast<CastExpr>(expr)) {
    processCastExpr(castExpr);
  } else if (const auto *parenExpr = llvm::dyn_cast<ParenExpr>(expr)) {
    processParenExpr(parenExpr);
  } else if (const auto *arraySubscript =
                 llvm::dyn_cast<ArraySubscriptExpr>(expr)) {
    processArraySubscriptExpr(arraySubscript);
  } else if (const auto *initList = llvm::dyn_cast<InitListExpr>(expr)) {
    processInitListExpr(initList);
  } else if (const auto *traitExpr =
                 llvm::dyn_cast<UnaryExprOrTypeTraitExpr>(expr)) {
    processUnaryExprOrTypeTraitExpr(traitExpr);
  }

  return findCachedExprId(expr);
}

QualType
ExprProcessor::getArrayNewAllocatedType(const CXXNewExpr *expr) const {
  if (!expr || !ast_context_ || !expr->isArray())
    return QualType();

  const QualType elementType = expr->getAllocatedType();
  const auto arraySize = expr->getArraySize();
  if (arraySize) {
    if (const auto constantSize =
            (*arraySize)->getIntegerConstantExpr(*ast_context_)) {
      return ast_context_->getConstantArrayType(
          elementType, *constantSize, *arraySize,
          ArraySizeModifier::Normal, 0);
    }
  }

  return ast_context_->getIncompleteArrayType(
      elementType, ArraySizeModifier::Normal, 0);
}

int ExprProcessor::classifyAllocatorForm(const CXXNewExpr *expr) const {
  if (!expr)
    return 0;
  if (expr->passAlignment())
    return 1;

  const FunctionDecl *allocator = expr->getOperatorNew();
  if (!allocator)
    return 0;
  for (const ParmVarDecl *param : allocator->parameters()) {
    if (param->getType()->isAlignValT() ||
        param->getType().getCanonicalType()->isAlignValT())
      return 1;
  }
  return 0;
}

int ExprProcessor::classifyDeallocatorForm(const FunctionDecl *decl) const {
  if (!decl || !ast_context_)
    return 0;

  int form = decl->isDestroyingOperatorDelete() ? 4 : 0;
  for (const ParmVarDecl *param : decl->parameters()) {
    const QualType paramType = param->getType();
    if (ast_context_->hasSameType(paramType, ast_context_->getSizeType()))
      form |= 1;
    if (paramType->isAlignValT() ||
        paramType.getCanonicalType()->isAlignValT())
      form |= 2;
  }
  return form;
}

bool ExprProcessor::requiresRuntimeDeallocatorSelection(
    const CXXDeleteExpr *expr) const {
  if (!expr || expr->isGlobalDelete() || expr->isArrayForm())
    return false;
  const QualType destroyedType = expr->getDestroyedType();
  if (destroyedType.isNull())
    return false;
  const CXXRecordDecl *record = destroyedType->getAsCXXRecordDecl();
  if (!record)
    return false;
  const CXXRecordDecl *definition = record->getDefinition();
  if (!definition)
    return false;
  record = definition;
  const CXXDestructorDecl *destructor = record->getDestructor();
  return destructor && destructor->isVirtual() && !record->isEffectivelyFinal();
}

bool ExprProcessor::hasTriviallyDestructibleDestroyedType(
    const CXXDeleteExpr *expr) const {
  if (!expr)
    return false;
  const QualType destroyedType = expr->getDestroyedType();
  return !destroyedType.isNull() &&
         destroyedType.isDestructedType() == QualType::DK_none;
}

void ExprProcessor::recordAllocator(int exprId, const CXXNewExpr *expr) {
  if (exprId < 0 || !expr || !function_processor_)
    return;
  const int functionId =
      function_processor_->resolveFunctionReference(expr->getOperatorNew());
  if (functionId < 0)
    return;
  DbModel::ExprAllocator allocatorModel = {
      exprId, functionId, classifyAllocatorForm(expr)};
  STG.insertClassObj(allocatorModel);
}

void ExprProcessor::recordDeallocator(int exprId, const FunctionDecl *decl) {
  if (exprId < 0 || !decl || !function_processor_)
    return;
  const int functionId = function_processor_->resolveFunctionReference(decl);
  if (functionId < 0)
    return;
  DbModel::ExprDeallocator deallocatorModel = {
      exprId, functionId, classifyDeallocatorForm(decl)};
  STG.insertClassObj(deallocatorModel);
}

void ExprProcessor::processCXXNewExpr(const CXXNewExpr *expr) {
  if (!canProcessExprForReference(expr) || findCachedExprId(expr) >= 0)
    return;

  const ExprKind kind =
      expr->isArray() ? ExprKind::NEW_ARRAY_EXPR : ExprKind::NEW_EXPR;
  const int exprId =
      processBaseExpr(const_cast<CXXNewExpr *>(expr), kind);
  if (exprId < 0)
    return;

  QualType allocatedType = expr->getAllocatedType();
  if (expr->isArray()) {
    // The reconstructed outer array is not part of the traversed TypeLoc.
    // Materialize its element first so nested fixed arrays resolve immediately.
    if (type_processor_)
      type_processor_->processType(allocatedType.getTypePtr());
    allocatedType = getArrayNewAllocatedType(expr);
  }
  if (!allocatedType.isNull() && type_processor_) {
    const int typeId = type_processor_->processType(allocatedType.getTypePtr());
    if (typeId >= 0) {
      if (expr->isArray()) {
        DbModel::NewArrayAllocatedType allocatedTypeModel = {exprId, typeId};
        STG.insertClassObj(allocatedTypeModel);
      } else {
        DbModel::NewAllocatedType allocatedTypeModel = {exprId, typeId};
        STG.insertClassObj(allocatedTypeModel);
      }
    }
  }

  recordAllocator(exprId, expr);
  if (expr->getOperatorDelete())
    recordDeallocator(exprId, expr->getOperatorDelete());

  if (const Expr *initializer = expr->getInitializer())
    recordExprParent(initializer, 1, exprId);
  if (const auto arraySize = expr->getArraySize();
      arraySize &&
      !(*arraySize)->isIntegerConstantExpr(*ast_context_))
    recordExprParent(*arraySize, 2, exprId);
}

void ExprProcessor::processCXXDeleteExpr(const CXXDeleteExpr *expr) {
  if (!canProcessExprForReference(expr) || findCachedExprId(expr) >= 0)
    return;

  const ExprKind kind = expr->isArrayForm() ? ExprKind::DELETE_ARRAY_EXPR
                                            : ExprKind::DELETE_EXPR;
  const int exprId =
      processBaseExpr(const_cast<CXXDeleteExpr *>(expr), kind);
  if (exprId < 0)
    return;

  if (!requiresRuntimeDeallocatorSelection(expr))
    recordDeallocator(exprId, expr->getOperatorDelete());
  if (hasTriviallyDestructibleDestroyedType(expr))
    recordExprParent(expr->getArgument(), 3, exprId);
}

const clang::Expr *
ExprProcessor::normalizeMainTreeExpr(const clang::Expr *expr) const {
  if (!expr)
    return nullptr;

  // CodeQL keeps conversions (including parentheses) outside the main
  // expression tree. P11 will model those through exprconv.
  return expr->IgnoreParenCasts();
}

int ExprProcessor::getOrProcessMainTreeExprId(const clang::Expr *expr) {
  return getOrProcessExprId(normalizeMainTreeExpr(expr));
}

void ExprProcessor::recordExprParent(const clang::Expr *child, int childIndex,
                                     int parentId) {
  if (parentId < 0)
    return;

  const int childId = getOrProcessMainTreeExprId(child);
  if (childId < 0)
    return;

  recordExprParentId(childId, childIndex, parentId);
}

void ExprProcessor::recordExprParentId(int childId, int childIndex,
                                       int parentId) {
  if (childId < 0 || parentId < 0)
    return;

  const std::string edgeKey = std::to_string(childId) + ":" +
                              std::to_string(childIndex) + ":" +
                              std::to_string(parentId);
  if (!recorded_parent_edges_.insert(edgeKey).second)
    return;

  DbModel::ExprParent parentModel = {childId, childIndex, parentId};
  STG.insertClassObj(parentModel);
}

void ExprProcessor::recordCallChildren(const CallExpr *expr, int parentId) {
  if (!expr || parentId < 0)
    return;

  if (expr->getDirectCallee()) {
    if (const auto *memberCall = llvm::dyn_cast<CXXMemberCallExpr>(expr)) {
      recordExprParent(memberCall->getImplicitObjectArgument(), -1, parentId);
    }
    for (unsigned index = 0; index < expr->getNumArgs(); ++index)
      recordExprParent(expr->getArg(index), static_cast<int>(index), parentId);
    return;
  }

  // Indirect calls expose the callee as child zero and shift arguments by one.
  recordExprParent(expr->getCallee(), 0, parentId);
  for (unsigned index = 0; index < expr->getNumArgs(); ++index)
    recordExprParent(expr->getArg(index), static_cast<int>(index + 1),
                     parentId);
}

int ExprProcessor::processBaseExpr(Expr *expr, ExprKind exprKind) {
  if (int cachedId = findCachedExprId(expr); cachedId != -1)
    return cachedId;

  KeyType exprKey = KeyGen::Expr_::makeKey(expr, ast_context_);
  LocIdPair *locIdPair = SrcLocRecorder::processExpr(expr, ast_context_);

  DbModel::Expr exprModel = {GENID(Expr), static_cast<int>(exprKind),
                             locIdPair->spec_id};

  INSERT_EXPR_CACHE(exprKey, exprModel.id);
  STG.insertClassObj(exprModel);
  recordExprType(expr, exprModel.id);
  return exprModel.id;
}

int ExprProcessor::processFunctionReference(const DeclRefExpr *expr) {
  if (!expr)
    return -1;
  if (const int cachedId = findCachedExprId(expr); cachedId >= 0)
    return cachedId;

  const auto *functionDecl = llvm::dyn_cast<FunctionDecl>(expr->getDecl());
  if (!functionDecl)
    return -1;

  const int exprId = processBaseExpr(const_cast<DeclRefExpr *>(expr),
                                     ExprKind::ROUTINEEXPR);
  const KeyType functionKey =
      KeyGen::Function::makeKey(functionDecl, ast_context_);
  if (auto functionId = SEARCH_FUNCTION_CACHE(functionKey)) {
    DbModel::FunBind bindModel = {exprId, *functionId};
    STG.insertClassObj(bindModel);
  } else {
    DbModel::FunBind bindModel = {exprId, -1};
    STG.insertClassObj(bindModel);
    PendingUpdate update{
        functionKey, CacheType::FUNCTION, [exprId](int resolvedId) {
          DbModel::FunBind resolvedModel = {exprId, resolvedId};
          STG.insertClassObj(resolvedModel);
        }};
    DependencyManager::instance().addDependency(update);
  }
  return exprId;
}

int ExprProcessor::processThisExpr(const CXXThisExpr *expr) {
  if (!expr)
    return -1;
  return processBaseExpr(const_cast<CXXThisExpr *>(expr),
                         ExprKind::THISACCESS);
}

int ExprProcessor::getOrProcessConversionSourceId(const Expr *expr) {
  if (!expr)
    return -1;

  if (const int exprId = getOrProcessExprId(expr); exprId >= 0)
    return exprId;

  if (const auto *memberExpr = llvm::dyn_cast<MemberExpr>(expr)) {
    processMemberExpr(memberExpr);
    return findCachedExprId(memberExpr);
  }

  if (const auto *declRef = llvm::dyn_cast<DeclRefExpr>(expr))
    return processFunctionReference(declRef);

  return -1;
}

void ExprProcessor::recordExprType(const Expr *expr, int exprId) {
  if (!expr || exprId < 0 || !type_processor_)
    return;

  const QualType expressionType = expr->getType();
  if (expressionType.isNull())
    return;

  const int typeId = type_processor_->processType(expressionType.getTypePtr());
  if (typeId < 0)
    return;

  if (specifier_processor_)
    specifier_processor_->processTypeQualifiers(typeId, expressionType);

  int valueCategory = 1;
  if (expr->isXValue())
    valueCategory = 2;
  else if (expr->isLValue())
    valueCategory = 3;

  DbModel::ExprType typeModel = {exprId, typeId, valueCategory};
  STG.insertClassObj(typeModel);
}

void ExprProcessor::processDeclRef(DeclRefExpr *expr) {
  if (findCachedExprId(expr) != -1)
    return;

  ValueDecl *valueDecl = expr->getDecl();

  if (auto *VD = dyn_cast<clang::VarDecl>(valueDecl)) {
    // Original behavior: create VARACCESS expr + VarBind
    int exprId = processBaseExpr(expr, ExprKind::VARACCESS);

    KeyType varKey = KeyGen::Var::makeKey(VD, ast_context_);
    LOG_DEBUG << "Searching variable cache for " << varKey << std::endl;
    int cachedVarId = -1;
    if (auto cachedId = SEARCH_VARIABLE_CACHE(varKey))
      cachedVarId = *cachedId;
    LOG_DEBUG << "Found variable ID: " << cachedVarId << std::endl;

    DbModel::VarBind varBindModel = {exprId, cachedVarId};
    STG.insertClassObj(varBindModel);
  } else if (llvm::isa<clang::NonTypeTemplateParmDecl>(valueDecl)) {
    // P3d: create VARACCESS expr for NonTypeTemplateParmDecl refs so they
    // have an @expr identity (needed for concept_template_argument_value).
    processBaseExpr(expr, ExprKind::VARACCESS);
  }
  // All other decl types (FunctionDecl, EnumConstantDecl, ConceptDecl, etc.)
  // are intentionally skipped — they are represented through their own paths
  // (CallExpr + FunBind, enumconstants, etc.) and should not become VARACCESS.
}

void ExprProcessor::processMemberExpr(const MemberExpr *expr) {
  if (!expr || findCachedExprId(expr) != -1)
    return;

  const int exprId =
      processBaseExpr(const_cast<MemberExpr *>(expr), ExprKind::CALLEXPR);
  recordExprParent(expr->getBase(), -1, exprId);

  const IsCallKind readKind = expr->isArrow() ? IsCallKind::MBRPTRREADEXPR
                                               : IsCallKind::MBRREADEXPR;
  DbModel::IsCall isCallModel = {exprId, static_cast<int>(readKind)};
  STG.insertClassObj(isCallModel);
}

void ExprProcessor::processUnaryOperator(const UnaryOperator *op) {
  if (findCachedExprId(op) != -1)
    return;

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

  const int parentId =
      processBaseExpr(const_cast<UnaryOperator *>(op), exprType);
  recordExprParent(op->getSubExpr(), 0, parentId);

  // 递归处理子表达式
  // Traverse(op->getSubExpr());
}

void ExprProcessor::processBinaryOperator(const BinaryOperator *op) {
  if (findCachedExprId(op) != -1)
    return;

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
    if (const int parentId = findCachedExprId(op); parentId != -1) {
      recordExprParent(op->getLHS(), 0, parentId);
      recordExprParent(op->getRHS(), 1, parentId);
    }
    return;
  default:
    return;
  }

  const int parentId =
      processBaseExpr(const_cast<BinaryOperator *>(op), expr_type);
  recordExprParent(op->getLHS(), 0, parentId);
  recordExprParent(op->getRHS(), 1, parentId);

  // Traverse(op->getLHS());
  // Traverse(op->getRHS());
}

void ExprProcessor::processConditionalOperator(const ConditionalOperator *op) {
  if (findCachedExprId(op) != -1)
    return;

  const int parentId = processBaseExpr(const_cast<ConditionalOperator *>(op),
                                       ExprKind::CONDITIONALEXPR);
  recordExprParent(op->getCond(), 0, parentId);
  recordExprParent(op->getTrueExpr(), 1, parentId);
  recordExprParent(op->getFalseExpr(), 2, parentId);
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
  if (findCachedExprId(literal) != -1)
    return;

  int exprId =
      processBaseExpr(const_cast<StringLiteral *>(literal), ExprKind::LITERAL);

  std::string value = literal->getString().str();
  std::string text = "\"" + value + "\"";

  processLiteralValue(value, text, exprId);
}

void ExprProcessor::processIntegerLiteral(const IntegerLiteral *literal) {
  processAttributeIntegerLiteral(literal);
}

int ExprProcessor::processAttributeIntegerLiteral(
    const IntegerLiteral *literal) {
  if (!literal)
    return -1;

  KeyType exprKey = KeyGen::Expr_::makeKey(literal, ast_context_);
  if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
    return *cachedId;

  int exprId =
      processBaseExpr(const_cast<IntegerLiteral *>(literal), ExprKind::LITERAL);

  std::string value = std::to_string(literal->getValue().getSExtValue());
  std::string text = value;

  processLiteralValue(value, text, exprId);
  return exprId;
}

void ExprProcessor::processFloatingLiteral(const FloatingLiteral *literal) {
  if (findCachedExprId(literal) != -1)
    return;

  int exprId = processBaseExpr(const_cast<FloatingLiteral *>(literal),
                               ExprKind::LITERAL);

  llvm::SmallVector<char, 16> buffer;
  literal->getValue().toString(buffer);
  std::string value(buffer.begin(), buffer.end());
  std::string text = value;

  processLiteralValue(value, text, exprId);
}

void ExprProcessor::processCharacterLiteral(const CharacterLiteral *literal) {
  if (findCachedExprId(literal) != -1)
    return;

  int exprId = processBaseExpr(const_cast<CharacterLiteral *>(literal),
                               ExprKind::LITERAL);

  std::string value = std::to_string(literal->getValue());
  std::string text =
      "'" + std::string(1, static_cast<char>(literal->getValue())) + "'";

  processLiteralValue(value, text, exprId);
}

void ExprProcessor::processBoolLiteral(const CXXBoolLiteralExpr *literal) {
  if (findCachedExprId(literal) != -1)
    return;

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

void ExprProcessor::processCallExpr(const CallExpr *expr) {
  if (findCachedExprId(expr) != -1)
    return;

  int exprId =
      processBaseExpr(const_cast<CallExpr *>(expr), ExprKind::CALLEXPR);
  recordCallChildren(expr, exprId);

  // Get the called function
  const FunctionDecl *callee = expr->getDirectCallee();
  if (callee) {
    // Generate key for the called function
    KeyType funcKey = KeyGen::Function::makeKey(callee, ast_context_);
    LOG_DEBUG << "CallExpr function key: " << funcKey << std::endl;

    // Check if function is in cache
    if (auto cachedFuncId = SEARCH_FUNCTION_CACHE(funcKey)) {
      // Create FunBind record linking expression to function
      DbModel::FunBind funBindModel = {exprId, *cachedFuncId};
      STG.insertClassObj(funBindModel);
    } else {
      // Function not in cache, create placeholder and add dependency
      DbModel::FunBind funBindModel = {exprId, -1};
      STG.insertClassObj(funBindModel);

      PendingUpdate update{
          funcKey, CacheType::FUNCTION, [exprId](int resolvedId) {
            DbModel::FunBind updated_record = {exprId, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // Create IsCall record to identify this as a call expression
  DbModel::IsCall isCallModel = {exprId,
                                 static_cast<int>(IsCallKind::MBRCALLEXPR)};
  STG.insertClassObj(isCallModel);
}

void ExprProcessor::processCastTypes(const CastExpr *castExpr) {
  if (!castExpr || !type_processor_)
    return;

  const QualType sourceType = castExpr->getSubExpr()->getType();
  const QualType targetType = castExpr->getType();
  const int sourceTypeId = type_processor_->processType(sourceType.getTypePtr());
  const int targetTypeId = type_processor_->processType(targetType.getTypePtr());

  if (specifier_processor_) {
    if (sourceTypeId >= 0)
      specifier_processor_->processTypeQualifiers(sourceTypeId, sourceType);
    if (targetTypeId >= 0)
      specifier_processor_->processTypeQualifiers(targetTypeId, targetType);
  }
}

void ExprProcessor::recordExprConv(int convertedId, int conversionId) {
  if (convertedId < 0 || conversionId < 0)
    return;
  DbModel::ExprConv conversionModel = {convertedId, conversionId};
  STG.insertClassObj(conversionModel);
}

void ExprProcessor::recordExprIsLoad(int exprId) {
  if (exprId < 0 || !recorded_loads_.insert(exprId).second)
    return;
  DbModel::ExprIsLoad loadModel = {exprId};
  STG.insertClassObj(loadModel);
}

void ExprProcessor::recordCompGenerated(int exprId) {
  if (exprId < 0)
    return;
  DbModel::CompGenerated generatedModel = {exprId};
  STG.insertClassObj(generatedModel);
}

void ExprProcessor::recordConversionKind(int exprId, int kind) {
  if (exprId < 0)
    return;
  DbModel::ConversionKind kindModel = {exprId, kind};
  STG.insertClassObj(kindModel);
}

ExprKind ExprProcessor::classifyCastExprKind(const CastExpr *castExpr) const {
  if (llvm::isa<CXXStaticCastExpr>(castExpr))
    return ExprKind::STATIC_CAST;
  if (llvm::isa<CXXReinterpretCastExpr>(castExpr))
    return ExprKind::REINTERPRET_CAST;
  if (llvm::isa<CXXConstCastExpr>(castExpr))
    return ExprKind::CONST_CAST;
  if (llvm::isa<CXXDynamicCastExpr>(castExpr))
    return ExprKind::DYNAMIC_CAST;
  return ExprKind::C_STYLE_CAST;
}

int ExprProcessor::classifyConversionKind(const CastExpr *castExpr) const {
  if (!castExpr)
    return 0;

  const QualType sourceType = castExpr->getSubExpr()->getType();
  const QualType targetType = castExpr->getType();
  if (targetType->isBooleanType())
    return 1;

  switch (castExpr->getCastKind()) {
  case CK_DerivedToBase:
  case CK_UncheckedDerivedToBase:
    return 2;
  case CK_BaseToDerived:
    return 3;
  case CK_DerivedToBaseMemberPointer:
    return 4;
  case CK_BaseToDerivedMemberPointer:
    return 5;
  default:
    break;
  }

  const bool sourceClass = sourceType->isRecordType();
  const bool targetClass = targetType->isRecordType();
  if (sourceClass && targetClass && castExpr->getSubExpr()->isGLValue() &&
      castExpr->isGLValue())
    return 6;

  if (sourceClass && targetClass && castExpr->getSubExpr()->isPRValue() &&
      castExpr->isPRValue() && ast_context_->hasSameUnqualifiedType(
                                      sourceType, targetType))
    return 7;

  return 0;
}

int ExprProcessor::processCastExpr(const CastExpr *castExpr) {
  if (!castExpr || !castExpr->getSubExpr() ||
      llvm::isa<BuiltinBitCastExpr>(castExpr) ||
      castExpr->isTypeDependent() || castExpr->isValueDependent() ||
      castExpr->isInstantiationDependent() ||
      castExpr->containsUnexpandedParameterPack())
    return -1;

  if (const int cachedId = findCachedExprId(castExpr); cachedId != -1)
    return cachedId;

  processCastTypes(castExpr);
  const int convertedId =
      getOrProcessConversionSourceId(castExpr->getSubExpr());
  if (convertedId < 0)
    return -1;

  const auto *implicitCast = llvm::dyn_cast<ImplicitCastExpr>(castExpr);
  if (implicitCast && implicitCast->getCastKind() == CK_LValueToRValue) {
    int loadExprId = getOrProcessMainTreeExprId(implicitCast->getSubExpr());
    if (loadExprId < 0)
      loadExprId = convertedId;
    if (loadExprId < 0)
      return -1;
    INSERT_EXPR_CACHE(KeyGen::Expr_::makeKey(castExpr, ast_context_),
                      convertedId);
    recordExprIsLoad(loadExprId);
    return convertedId;
  }

  ExprKind exprKind = classifyCastExprKind(castExpr);
  if (implicitCast && implicitCast->getCastKind() == CK_ArrayToPointerDecay)
    exprKind = ExprKind::ARRAY_TO_POINTER;

  const int conversionId =
      processBaseExpr(const_cast<CastExpr *>(castExpr), exprKind);
  recordExprConv(convertedId, conversionId);

  if (implicitCast) {
    recordCompGenerated(conversionId);
    if (exprKind == ExprKind::C_STYLE_CAST)
      recordConversionKind(conversionId, classifyConversionKind(castExpr));
  } else {
    recordConversionKind(conversionId, classifyConversionKind(castExpr));
  }

  return conversionId;
}

int ExprProcessor::processParenExpr(const ParenExpr *parenExpr) {
  if (!parenExpr || !parenExpr->getSubExpr() ||
      parenExpr->isTypeDependent() || parenExpr->isValueDependent() ||
      parenExpr->isInstantiationDependent() ||
      parenExpr->containsUnexpandedParameterPack())
    return -1;

  if (const int cachedId = findCachedExprId(parenExpr); cachedId != -1)
    return cachedId;

  const int convertedId = getOrProcessExprId(parenExpr->getSubExpr());
  if (convertedId < 0)
    return -1;

  const int conversionId =
      processBaseExpr(const_cast<ParenExpr *>(parenExpr), ExprKind::PAREXPR);
  recordExprConv(convertedId, conversionId);
  return conversionId;
}

void ExprProcessor::processArraySubscriptExpr(const ArraySubscriptExpr *expr) {
  if (findCachedExprId(expr) != -1)
    return;

  const int parentId = processBaseExpr(const_cast<ArraySubscriptExpr *>(expr),
                                       ExprKind::SUBSCRIPTEXPR);
  recordExprParent(expr->getBase(), 0, parentId);
  recordExprParent(expr->getIdx(), 1, parentId);
}

void ExprProcessor::processInitListExpr(const InitListExpr *expr) {
  if (const auto *semantic = expr->getSemanticForm())
    expr = semantic;
  if (findCachedExprId(expr) != -1)
    return;

  int exprId = processBaseExpr(const_cast<InitListExpr *>(expr),
                               ExprKind::BRACED_INIT_LIST);

  for (unsigned index = 0; index < expr->getNumInits(); ++index)
    recordExprParent(expr->getInit(index), static_cast<int>(index), exprId);

  QualType initType = expr->getType();

  if (initType->isArrayType()) {
    recordAggregateArrayInit(exprId, expr);
  } else if (initType->isRecordType()) {
    recordAggregateFieldInit(exprId, expr);
  } else {
    LOG_DEBUG << "InitListExpr for neither array nor record type" << std::endl;
  }
}

void ExprProcessor::recordAggregateArrayInit(int initListExprId,
                                             const InitListExpr *expr) {
  unsigned numInits = expr->getNumInits();

  for (unsigned i = 0; i < numInits; ++i) {
    const Expr *init = expr->getInit(i);
    if (!init) continue;

    KeyType initKey = KeyGen::Expr_::makeKey(init, ast_context_);
    int initExprId = SEARCH_EXPR_CACHE(initKey).value_or(-1);

    auto initModel =
        std::make_shared<DbModel::AggregateArrayInit>(
            DbModel::AggregateArrayInit{
                initListExprId,      // aggregate (@aggregateliteral ref)
                initExprId,          // initializer (@expr ref)
                static_cast<int>(i), // element_index (int ref)
                static_cast<int>(i)  // position (int ref)
            });
    STG.insertClassObj(*initModel);

    if (initExprId == -1) {
      PendingUpdate update{
          initKey, CacheType::EXPR, [initModel](int resolvedExprId) {
            initModel->initializer = resolvedExprId;
            STG.insertClassObj(*initModel);
          }};
      DependencyManager::instance().addDependency(update);
      LOG_WARNING << "Init expression not in cache for array index " << i
                  << std::endl;
    }
  }
}

void ExprProcessor::recordAggregateFieldInit(int initListExprId,
                                             const InitListExpr *expr) {
  const RecordType *recordType = expr->getType()->getAs<RecordType>();
  if (!recordType || !recordType->getDecl()) {
    LOG_WARNING << "Cannot get RecordType or RecordDecl for InitListExpr" << std::endl;
    return;
  }

  const RecordDecl *recordDecl = recordType->getDecl();
  const unsigned numInits = expr->getNumInits();
  // Semantic initializer slots start with direct bases, which are not fields.
  // Unnamed bit-fields consume no slot, and a union has only its active field.
  unsigned initIndex = 0;
  if (const auto *cxxRecord = llvm::dyn_cast<CXXRecordDecl>(recordDecl))
    initIndex = cxxRecord->getNumBases();
  const FieldDecl *unionField = recordDecl->isUnion()
                                   ? expr->getInitializedFieldInUnion()
                                   : nullptr;
  for (const auto *field : recordDecl->fields()) {
    if (field->isUnnamedBitField() ||
        (recordDecl->isUnion() && field != unionField))
      continue;
    if (initIndex >= numInits)
      break;

    const unsigned position = initIndex++;
    const Expr *init = expr->getInit(position);
    if (!init)
      continue;

    // 获取初始化表达式 ID
    KeyType initKey = KeyGen::Expr_::makeKey(init, ast_context_);
    int initExprId = SEARCH_EXPR_CACHE(initKey).value_or(-1);

    // 使用标准的键生成器查找字段 ID (@membervariable ref)
    KeyType fieldKey = KeyGen::Var::makeKey(field, ast_context_);
    int fieldId = SEARCH_MEMBERVAR_CACHE(fieldKey).value_or(-1);

    auto initModel =
        std::make_shared<DbModel::AggregateFieldInit>(
            DbModel::AggregateFieldInit{
                initListExprId, // aggregate (@aggregateliteral ref)
                initExprId,     // initializer (@expr ref)
                fieldId,        // field (@membervariable ref)
                static_cast<int>(position) // semantic initializer position
            });
    STG.insertClassObj(*initModel);

    if (initExprId == -1) {
      PendingUpdate update{
          initKey, CacheType::EXPR, [initModel](int resolvedExprId) {
            initModel->initializer = resolvedExprId;
            STG.insertClassObj(*initModel);
          }};
      DependencyManager::instance().addDependency(update);
    }

    if (fieldId == -1) {
      PendingUpdate update{
          fieldKey, CacheType::MEMBERVERY, [initModel](int resolvedFieldId) {
            initModel->field = resolvedFieldId;
            STG.insertClassObj(*initModel);
          }};
      DependencyManager::instance().addDependency(update);

      std::string fieldName = field->getNameAsString();
      std::string recordName = recordDecl->getNameAsString();
      LOG_DEBUG << "Added dependency for MemberVar '" << fieldName
                << "' in '" << recordName << "'" << std::endl;
    }
  }
}

void ExprProcessor::processUnaryExprOrTypeTraitExpr(const UnaryExprOrTypeTraitExpr *expr) {
  if (findCachedExprId(expr) != -1)
    return;

  UnaryExprOrTypeTrait kind = expr->getKind();

  if (kind != UETT_SizeOf && kind != UETT_AlignOf) {
    LOG_DEBUG << "Unsupported UnaryExprOrTypeTrait kind" << std::endl;
    return;
  }

  ExprKind exprKind = (kind == UETT_SizeOf) ? ExprKind::RUNTIME_SIZEOF : ExprKind::RUNTIME_ALIGNOF;

  int exprId = processBaseExpr(const_cast<UnaryExprOrTypeTraitExpr *>(expr), exprKind);
  if (!expr->isArgumentType())
    recordExprParent(expr->getArgumentExpr(), 0, exprId);
  recordSizeOfBind(exprId, expr);
}

void ExprProcessor::recordSizeOfBind(int exprId, const UnaryExprOrTypeTraitExpr *expr) {
  if (expr->isArgumentType()) {
    // sizeof(type) or alignof(type)
    QualType typeQual = expr->getArgumentType();
    const Type *type = typeQual.getTypePtr();

    int typeId = -1;
    if (type_processor_) {
      typeId = type_processor_->processType(type);
    } else {
      LOG_WARNING << "TypeProcessor not available for sizeof/alignof type" << std::endl;
    }

    DbModel::SizeOfBind bindModel = {exprId, typeId};
    STG.insertClassObj(bindModel);
  } else {
    // sizeof expr or alignof expr
    // 需要从表达式中提取类型信息
    const Expr *argExpr = expr->getArgumentExpr();
    if (!argExpr) {
      LOG_WARNING << "UnaryExprOrTypeTraitExpr has null argument expression" << std::endl;
      return;
    }

    // 获取表达式的类型
    QualType typeQual = argExpr->getType();
    const Type *type = typeQual.getTypePtr();

    int typeId = -1;
    if (type_processor_) {
      typeId = type_processor_->processType(type);
    } else {
      LOG_WARNING << "TypeProcessor not available for sizeof/alignof expr type" << std::endl;
    }

    DbModel::SizeOfBind bindModel = {exprId, typeId};
    STG.insertClassObj(bindModel);
  }
}

int ExprProcessor::processConceptSpecializationExpr(
    const ConceptSpecializationExpr *expr, int conceptId) {
  if (!expr || conceptId == -1)
    return -1;

  KeyType exprKey = KeyGen::Expr_::makeKey(expr, ast_context_);
  if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
    return *cachedId;

  LocIdPair *locIdPair = SrcLocRecorder::processExpr(expr, ast_context_);
  DbModel::Expr exprModel = {conceptId,
                             static_cast<int>(ExprKind::CONCEPT_ID),
                             locIdPair->spec_id};

  INSERT_EXPR_CACHE(exprKey, exprModel.id);
  STG.insertClassObj(exprModel);
  recordExprType(expr, exprModel.id);
  return exprModel.id;
}

int ExprProcessor::processNonTypeTemplateParmDecl(
    const NonTypeTemplateParmDecl *decl) {
  if (!decl)
    return -1;

  KeyType exprKey =
      KeyGen::Expr_::makeKeyForNonTypeTemplateParm(decl, ast_context_);
  if (exprKey.empty())
    return -1;

  if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
    return *cachedId;

  int exprId = IDGenerator::generateId<DbModel::Expr>();
  LocIdPair *locIdPair = SrcLocRecorder::processExpr(decl, ast_context_);
  DbModel::Expr exprModel = {exprId,
                             static_cast<int>(ExprKind::NONTYPE_TEMPLATE_PARAMETER),
                             locIdPair->spec_id};

  INSERT_EXPR_CACHE(exprKey, exprModel.id);
  STG.insertClassObj(exprModel);
  return exprModel.id;
}
