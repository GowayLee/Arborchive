#include "core/processor/function_processor.h"
#include "db/storage_facade.h"
#include "model/db/function.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
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

  // Record @purefunction @function_deleted @function_defaulted
  // @function_prototyped
  recordBasicInfo(decl);

  // Record @function_entry_point stmt
  recordEntryPoint(decl);

  // Record @function_return_type
  recordReturnType(decl);

  // Record execption throw and noexcept
  recordException(decl);

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
