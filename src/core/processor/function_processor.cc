#include "core/processor/function_processor.h"
#include "core/processor/coroutine_helper.h"
#include "db/storage_facade.h"
#include "model/db/function.h"
#include "model/db/type.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/function.h"
#include "util/key_generator/stmt.h"
#include "util/key_generator/type.h"
#include "util/logger/macros.h"
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Basic/ExceptionSpecificationType.h>

Stmt *getFirstNonCompoundStmt(clang::Stmt *S);

// Create base function struct and return id
void FunctionProcessor::handleBaseFunc(const FunctionDecl *decl,
                                       const FuncType type) {
  _locIdPair = PROC_DEFT(cast<Decl>(decl), ast_context_);
  std::string name = decl->getNameAsString();
  DbModel::Function function = {_funcId = GENID(Function), name,
                                static_cast<int>(type)};

  // Insert Cache
  KeyType funcKey = KeyGen::Function::makeKey(decl, decl->getASTContext());
  LOG_DEBUG << "Function FunctionKey: " << funcKey << std::endl;
  INSERT_FUNCTION_CACHE(funcKey, _funcId);

  // Record @purefunction @function_deleted @function_defaulted
  // @function_prototyped
  recordBasicInfo(decl);

  // Record @function_entry_point stmt
  recordEntryPoint(decl);

  // Record @function_return_type
  recordReturnType(decl);

  // Record execption throw and noexcept
  recordException(decl);

  // Record function typedef
  recordTypedef(decl);

  // Record coroutine
  recordCoroutine(decl);

  DbModel::FunDecl fun_decl = {_funcDeclId = GENID(FunDecl), _funcId, _typeId,
                               name, _locIdPair->spec_id};

  STG.insertClassObj(function);
  STG.insertClassObj(fun_decl);
}

void FunctionProcessor::recordEntryPoint(const FunctionDecl *decl) const {
  if (!decl->hasBody()) // Skip if function has no body
    return;
  Stmt *body = decl->getBody();
  Stmt *entryStmt = getFirstNonCompoundStmt(body);
  if (entryStmt == nullptr) // Skip if entry statement is not found
    return;
  std::string stmtKey =
      KeyGen::Stmt_::makeKey(entryStmt, decl->getASTContext());
  LOG_DEBUG << "Function entry point StmtKey: " << stmtKey << std::endl;
  int stmtId = -1;
  if (auto cachedId = SEARCH_STMT_CACHE(stmtKey))
    stmtId = *cachedId;
  else {
    // LOG_DEBUG << "Stmt cache entry not found, push to pending model queue"
    //           << std::endl;
    // TODO: Dependency Manager...
  }
  DbModel::FuncEntryPt funcEntryPt = {_funcId, stmtId};
  STG.insertClassObj(funcEntryPt);
}

void FunctionProcessor::recordBasicInfo(const FunctionDecl *decl) const {
  if (decl->isPureVirtual()) {
    DbModel::PureFuncs func_pure = {_funcId};
    STG.insertClassObj(func_pure);
  }
  if (decl->isDeleted()) {
    DbModel::FuncDeleted func_deleted = {_funcId};
    STG.insertClassObj(func_deleted);
  }
  if (decl->isDefaulted()) {
    DbModel::FuncDefaulted func_default = {_funcId};
    STG.insertClassObj(func_default);
  }
  if (decl->hasPrototype()) {
    DbModel::FuncPrototyped func_prototype = {_funcId};
    STG.insertClassObj(func_prototype);
  }
  if (decl->isFunctionTemplateSpecialization()) {
    DbModel::FunSpecialized fun_specialized = {_funcId};
    STG.insertClassObj(fun_specialized);
  }
  if (decl->isImplicit()) {
    DbModel::FunImplicit fun_implicit = {_funcId};
    STG.insertClassObj(fun_implicit);
  }
}

void FunctionProcessor::recordReturnType(const FunctionDecl *decl) {
  // Check Type cache for Id for returnType
  KeyType typeKey =
      KeyGen::Type::makeKey(decl->getReturnType(), decl->getASTContext());
  LOG_DEBUG << "Function TypeKey: " << typeKey << std::endl;
  _typeId = -1;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    _typeId = *cachedId;
  else {
    // LOG_DEBUG << "Type cache entry not found, push to pending model queue"
    //           << std::endl;
    // TODO: Dependency Manager...
  }
  DbModel::FuncRetType func_ret_type = {_funcId, _typeId};
  STG.insertClassObj(func_ret_type);
}

void FunctionProcessor::recordException(const clang::FunctionDecl *decl) const {
  const auto *typeSrcInfo = decl->getTypeSourceInfo();
  if (!typeSrcInfo)
    return;
  const auto *funcProtoType =
      typeSrcInfo->getType()->getAs<FunctionProtoType>();
  if (!funcProtoType)
    return;
  // 获取处理不同类型的异常规范
  switch (funcProtoType->getExceptionSpecType()) {
  case EST_DynamicNone: { // fun_decl_empty_throws
    DbModel::FunDeclEmptyThrow eptThrow = {_funcDeclId};
    STG.insertClassObj(eptThrow);
    break;
  }
  case EST_Dynamic: { // fun_decl_throws
    // 处理 throw(Type1, Type2, ...)
    int index = 0;
    for (const auto &qt : funcProtoType->exceptions()) {
      KeyType typeKey = KeyGen::Type::makeKey(qt, decl->getASTContext());
      int typeId = -1;
      if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
        typeId = *cachedId;
      else {
        // TODO: Dependency Manager...
      }
      DbModel::FunDeclThrow funDeclThrow = {_funcDeclId, index++, typeId};
      STG.insertClassObj(funDeclThrow);
    }
    break;
  }
  case EST_BasicNoexcept: { // fun_decl_noexcept
    DbModel::FunDeclEmptyNoexcept eptNoexcept = {_funcDeclId};
    STG.insertClassObj(eptNoexcept);
    break;
  }
  case EST_DependentNoexcept: {
    const Expr *noexceptExpr = funcProtoType->getNoexceptExpr();
    if (noexceptExpr) {
      KeyType exprKey =
          KeyGen::Expr_::makeKey(noexceptExpr, decl->getASTContext());
      LOG_DEBUG << "Expr key: " << exprKey << std::endl;
      int exprId = -1;
      if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
        exprId = *cachedId;
      DbModel::FunDeclNoexcept funDeclNoexcept = {_funcDeclId, exprId};
      STG.insertClassObj(funDeclNoexcept);
      // TODO: Assume RecursiveVisitor will first visit noexcept node then,
      // inside expr node. So, hereby, we only caculate exprKey, after inside
      // expr node is visited, call Dependency Manager to fill real id by
      // exprKey
    } else {
      // If noexcepExpr is a nullptr, treat as fun_decl_empty_noexcept
      DbModel::FunDeclEmptyNoexcept eptNoexcept = {_funcDeclId};
      STG.insertClassObj(eptNoexcept);
    }
    break;
  }
  case EST_NoexceptTrue: {
    // Need to create an expr entity, literal true
    const Expr *noexceptExpr = funcProtoType->getNoexceptExpr();
    if (noexceptExpr) {
      KeyType exprKey =
          KeyGen::Expr_::makeKey(noexceptExpr, decl->getASTContext());
      LOG_DEBUG << "Expr key: " << exprKey << std::endl;
      int exprId = -1;
      if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
        exprId = *cachedId;
      DbModel::FunDeclNoexcept funDeclNoexcept = {_funcDeclId, exprId};
      STG.insertClassObj(funDeclNoexcept);
    }
    break;
  }
  case EST_NoexceptFalse: {
    // Need to create an expr entity, literal false
    const Expr *noexceptExpr = funcProtoType->getNoexceptExpr();
    if (noexceptExpr) {
      KeyType exprKey =
          KeyGen::Expr_::makeKey(noexceptExpr, decl->getASTContext());
      LOG_DEBUG << "Expr key: " << exprKey << std::endl;
      int exprId = -1;
      if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
        exprId = *cachedId;
      DbModel::FunDeclNoexcept funDeclNoexcept = {_funcDeclId, exprId};
      STG.insertClassObj(funDeclNoexcept);
    }
    break;
  }
  default:
    break;
  }
}

void FunctionProcessor::recordTypedef(const FunctionDecl *decl) const {
  // 获取函数声明的类型
  QualType funcType = decl->getType();

  // 检查是否是typedef类型
  if (const TypedefType *TT = funcType->getAs<TypedefType>()) {
    TypedefNameDecl *TND = TT->getDecl();
    if (TND) {
      DbModel::UserType::KeyType userTypekey =
          KeyGen::Type::makeKey(TND, decl->getASTContext());
      LOG_DEBUG << "Function Typedef typeKey" << userTypekey << std::endl;
      int userTypeId = -1;
      if (auto cachedId = SEARCH_USERTYPE_CACHE(userTypekey))
        userTypeId = *cachedId;
      else {
        // TODO: Dependency Manager...
        // FIXME: Maybe function typedef is also the node which declares such
        // userType
      }
      DbModel::FunDeclTypedefType funDeclTypedefType = {_funcDeclId,
                                                        userTypeId};
      STG.insertClassObj(funDeclTypedefType);
    }
  } else if (const ElaboratedType *ET = funcType->getAs<ElaboratedType>()) {
    // 处理可能嵌套在ElaboratedType中的TypedefType
    if (const TypedefType *TT = ET->getNamedType()->getAs<TypedefType>()) {
      TypedefNameDecl *TND = TT->getDecl();
      if (TND) {
        DbModel::UserType::KeyType userTypekey =
            KeyGen::Type::makeKey(TND, decl->getASTContext());
        LOG_DEBUG << "Function Typedef typeKey" << userTypekey << std::endl;
        int userTypeId = -1;
        if (auto cachedId = SEARCH_USERTYPE_CACHE(userTypekey))
          userTypeId = *cachedId;
        else {
          // TODO: Dependency Manager...
          // FIXME: Maybe function typedef is also the node which declares such
          // userType
        }
        DbModel::FunDeclTypedefType funDeclTypedefType = {_funcDeclId,
                                                          userTypeId};
        STG.insertClassObj(funDeclTypedefType);
      }
    }
  }
}

void FunctionProcessor::recordCoroutine(const FunctionDecl *FD) {
  if (!isCoroutineFunction(FD))
    return;

  // 查找coroutine_traits模板
  ASTContext &Context = FD->getASTContext();
  auto *TraitsTemplate = findCoroutineTraitsTemplate(Context);

  if (!TraitsTemplate)
    return;

  // 获取traits类型
  QualType TraitsType = getCoroutineTraitsType(Context, FD, TraitsTemplate);

  if (TraitsType.isNull())
    return;

  KeyType typeKey = KeyGen::Type::makeKey(TraitsType, FD->getASTContext());
  LOG_DEBUG << "Coroutine Trait TypeKey: " << typeKey << std::endl;
  int typeId = -1;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    typeId = *cachedId;
  else {
    // TODO: Dependency Manager...
  }
  DbModel::Coroutine coroutine = {_funcId, typeId};
  STG.insertClassObj(coroutine);

  // Get coroutine_new
  // 获取并记录协程的new函数
  FunctionDecl *NewFD = getCoroutineNewFunction(FD, Context);
  if (NewFD) {
    KeyType newFuncKey = KeyGen::Function::makeKey(NewFD, FD->getASTContext());
    LOG_DEBUG << "Function FuncKey: " << newFuncKey << std::endl;
    int newFuncId = -1;
    if (auto cachedId = SEARCH_FUNCTION_CACHE(newFuncKey))
      newFuncId = *cachedId;
    else {
      // TODO: Dependency Manager...
    }
    DbModel::CoroutineNew couroutine_new = {_funcId, newFuncId};
    STG.insertClassObj(couroutine_new);
  }

  // Get coroutine_delete
  // 获取并记录协程的delete函数
  FunctionDecl *DelFD = getCoroutineDeleteFunction(FD, Context);
  if (DelFD) {
    KeyType delFuncKey = KeyGen::Function::makeKey(DelFD, FD->getASTContext());
    LOG_DEBUG << "Function FuncKey: " << delFuncKey << std::endl;
    int delFuncId = -1;
    if (auto cachedId = SEARCH_FUNCTION_CACHE(delFuncKey))
      delFuncId = *cachedId;
    else {
      // TODO: Dependency Manager...
    }
    DbModel::CoroutineDelete couroutine_delete = {_funcId, delFuncId};
    STG.insertClassObj(couroutine_delete);
  }
}

// Router to process functions of @operator @builtin_function
// @user_defined_function, @normal_function
void FunctionProcessor::routerProcess(const clang::FunctionDecl *decl) {
  auto kind = decl->getKind();
  // Return first, will be processed by other functions
  if (kind == Decl::CXXConstructor || kind == Decl::CXXDestructor ||
      kind == Decl::CXXConversion || kind == Decl::CXXDeductionGuide)
    return;

  // Determine isOperator
  if (decl->isOverloadedOperator()) {
    processOperatorFunc(cast<FunctionDecl>(decl));
    return;
  }

  // Determine isBuiltin

  // Check isBuiltin without nameValid
  if (decl->getBuiltinID() != 0) {
    processBuiltinFunc(cast<FunctionDecl>(decl));
    return;
  }

  // Check isBuiltin with nameValid
  if (decl->getIdentifier() != nullptr) {
    llvm::StringRef name = decl->getName();

    // Check is starts with "__builtin__"
    if (name.starts_with(
            "__builtin__")) { // FIXME: may be official API support?
      processBuiltinFunc(cast<FunctionDecl>(decl));
      return;
    }

    // Check isUserDefinedLiteral
    if (name.find("operator\"\"") == 0) { // FIXME: maybe official API support?
      processUserDefinedLiteral(cast<FunctionDecl>(decl));
      return;
    }
  } else {
    LOG_DEBUG << "Function has no identifier, skipping name-based checks"
              << std::endl;
  }

  // Otherwise, isNormalFunction
  processNormalFunc(cast<FunctionDecl>(decl));
}

void FunctionProcessor::processBuiltinFunc(const clang::FunctionDecl *decl) {
  handleBaseFunc(cast<FunctionDecl>(decl), FuncType::BUILDIN_FUNC);
}

void FunctionProcessor::processUserDefinedLiteral(
    const clang::FunctionDecl *decl) {

  handleBaseFunc(cast<FunctionDecl>(decl), FuncType::USER_DEFINED_LITERAL);
}

void FunctionProcessor::processOperatorFunc(const clang::FunctionDecl *decl) {

  handleBaseFunc(cast<FunctionDecl>(decl), FuncType::OPERATOR);
}

void FunctionProcessor::processNormalFunc(const clang::FunctionDecl *decl) {

  handleBaseFunc(cast<FunctionDecl>(decl), FuncType::NORM_FUNC);
}

void FunctionProcessor::processCXXConstructor(const CXXConstructorDecl *decl) {
  handleBaseFunc(cast<FunctionDecl>(decl), FuncType::CONSTRUCTOR);
}

void FunctionProcessor::processCXXDestructor(const CXXDestructorDecl *decl) {
  handleBaseFunc(cast<FunctionDecl>(decl), FuncType::DESTRUCTOR);
}

void FunctionProcessor::processCXXConversion(const CXXConversionDecl *decl) {
  handleBaseFunc(cast<FunctionDecl>(decl), FuncType::CONVERSION_FUNC);
}

void FunctionProcessor::processCXXDeductionGuide(
    const CXXDeductionGuideDecl *decl) {
  handleBaseFunc(cast<FunctionDecl>(decl), FuncType::DEDUCTION_GUIDE);
  KeyType key =
      KeyGen::Type::makeKey(decl->getDeducedTemplate(), decl->getASTContext());
  LOG_DEBUG << "Deduction Guide Key: " << key << std::endl;
  int userTypeId = -1;
  if (auto cachedId = SEARCH_USERTYPE_CACHE(key))
    userTypeId = *cachedId;
  else {
    // Handle dependency manage
  }
  // Since this function is called by RecursiveVisitor after executing
  // routerProcess() memeber variable _funcId still storages id of current
  // function
  DbModel::DeductionGuideForClass deducGuide = {_funcId, userTypeId};
  STG.insertClassObj(deducGuide);
}

Stmt *getFirstNonCompoundStmt(clang::Stmt *S) {
  if (!S)
    return nullptr;
  // Recursively process the first non-compound statement
  if (auto *CS = dyn_cast<clang::CompoundStmt>(S)) {
    if (CS->body_empty())
      return nullptr; // return nullptr if the compound statement is empty
    return getFirstNonCompoundStmt(*CS->body_begin());
  }
  // Non-compound statement directly returns itself
  return S;
}
