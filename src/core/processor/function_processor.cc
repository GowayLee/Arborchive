#include "core/processor/function_processor.h"
#include "core/processor/coroutine_helper.h"
#include "db/dependency_manager.h"
#include "db/storage_facade.h"
#include "model/db/element.h"
#include "model/db/function.h"
#include "model/db/type.h"
#include "util/id_generator.h"
#include "util/key_generator/element.h"
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
  _funcDeclId = GENID(FunDecl); // Generate ID early for dependency capturing

  // Insert Cache
  KeyType funcKey = KeyGen::Function::makeKey(decl, decl->getASTContext());
  LOG_DEBUG << "Function FunctionKey: " << funcKey << std::endl;
  INSERT_FUNCTION_CACHE(funcKey, _funcId);

  KeyType elementKey = KeyGen::Element::makeKeyFromFuncKey(funcKey);
  DbModel::ParameterizedElement parameterizedElement = {
      GENID(ParameterizedElement), _funcId,
      static_cast<int>(ElementType::FUNCTION)};
  INSERT_ELEMENT_CACHE(elementKey, _funcId);

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

  DbModel::FunDecl fun_decl = {_funcDeclId, _funcId, _typeId, name,
                               _locIdPair->spec_id};

  STG.insertClassObj(function);
  STG.insertClassObj(fun_decl);
  STG.insertClassObj(parameterizedElement);
}

void FunctionProcessor::recordEntryPoint(const FunctionDecl *decl) const {
  if (!decl->hasBody()) // Skip if function has no body
    return;
  Stmt *body = decl->getBody();
  Stmt *entryStmt = getFirstNonCompoundStmt(body);
  std::string stmtKey =
      KeyGen::Stmt_::makeKey(entryStmt, decl->getASTContext());
  LOG_DEBUG << "Function entry point StmtKey: " << stmtKey << std::endl;

  if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
    DbModel::FuncEntryPt funcEntryPt = {_funcId, *cachedId};
    STG.insertClassObj(funcEntryPt);
  } else {
    DbModel::FuncEntryPt funcEntryPt = {_funcId, -1};
    STG.insertClassObj(funcEntryPt);
    PendingUpdate update{
        stmtKey, CacheType::STMT, [_funcId = _funcId](int resolvedId) {
          DbModel::FuncEntryPt updated_record = {_funcId, resolvedId};
          STG.insertClassObj(updated_record);
        }};
    DependencyManager::instance().addDependency(update);
  }
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

  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey)) {
    _typeId = *cachedId;
  } else {
    _typeId = -1;
    // Register dependency for FunDecl table
    PendingUpdate funDeclUpdate{
        typeKey, CacheType::TYPE,
        [_funcDeclId = _funcDeclId, _funcId = _funcId,
         name = decl->getNameAsString(),
         spec_id = _locIdPair->spec_id](int resolvedId) {
          DbModel::FunDecl updated_record = {_funcDeclId, _funcId, resolvedId,
                                             name, spec_id};
          STG.insertClassObj(updated_record);
        }};
    DependencyManager::instance().addDependency(funDeclUpdate);
  }
  // This record always needs to be inserted, either with a real ID or a
  // dependency
  DbModel::FuncRetType func_ret_type = {_funcId, _typeId};
  STG.insertClassObj(func_ret_type);
  if (_typeId == -1) {
    PendingUpdate funcRetUpdate{
        typeKey, CacheType::TYPE, [_funcId = _funcId](int resolvedId) {
          DbModel::FuncRetType updated_record = {_funcId, resolvedId};
          STG.insertClassObj(updated_record);
        }};
    DependencyManager::instance().addDependency(funcRetUpdate);
  }
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
      if (auto cachedId = SEARCH_TYPE_CACHE(typeKey)) {
        DbModel::FunDeclThrow funDeclThrow = {_funcDeclId, index, *cachedId};
        STG.insertClassObj(funDeclThrow);
      } else {
        DbModel::FunDeclThrow funDeclThrow = {_funcDeclId, index, -1};
        STG.insertClassObj(funDeclThrow);
        PendingUpdate update{
            typeKey, CacheType::TYPE,
            [_funcDeclId = _funcDeclId, index = index](int resolvedId) {
              DbModel::FunDeclThrow updated_record = {_funcDeclId, index,
                                                      resolvedId};
              STG.insertClassObj(updated_record);
            }};
        DependencyManager::instance().addDependency(update);
      }
      index++;
    }
    break;
  }
  case EST_BasicNoexcept: { // fun_decl_noexcept
    DbModel::FunDeclEmptyNoexcept eptNoexcept = {_funcDeclId};
    STG.insertClassObj(eptNoexcept);
    break;
  }
  case EST_DependentNoexcept:
  case EST_NoexceptTrue:
  case EST_NoexceptFalse: {
    const Expr *noexceptExpr = funcProtoType->getNoexceptExpr();
    if (noexceptExpr) {
      KeyType exprKey =
          KeyGen::Expr_::makeKey(noexceptExpr, decl->getASTContext());
      LOG_DEBUG << "Expr key: " << exprKey << std::endl;
      if (auto cachedId = SEARCH_EXPR_CACHE(exprKey)) {
        DbModel::FunDeclNoexcept funDeclNoexcept = {_funcDeclId, *cachedId};
        STG.insertClassObj(funDeclNoexcept);
      } else {
        DbModel::FunDeclNoexcept funDeclNoexcept = {_funcDeclId, -1};
        STG.insertClassObj(funDeclNoexcept);
        PendingUpdate update{exprKey, CacheType::EXPR,
                             [_funcDeclId = _funcDeclId](int resolvedId) {
                               DbModel::FunDeclNoexcept updated_record = {
                                   _funcDeclId, resolvedId};
                               STG.insertClassObj(updated_record);
                             }};
        DependencyManager::instance().addDependency(update);
      }
    } else if (funcProtoType->getExceptionSpecType() == EST_DependentNoexcept) {
      // If noexcepExpr is a nullptr, treat as fun_decl_empty_noexcept
      DbModel::FunDeclEmptyNoexcept eptNoexcept = {_funcDeclId};
      STG.insertClassObj(eptNoexcept);
    }
    break;
  }
  default:
    break;
  }
}

void FunctionProcessor::recordTypedef(const FunctionDecl *decl) const {
  auto processTypedef = [&](const TypedefNameDecl *TND) {
    if (TND) {
      DbModel::UserType::KeyType userTypekey =
          KeyGen::Type::makeKey(TND, decl->getASTContext());
      LOG_DEBUG << "Function Typedef typeKey" << userTypekey << std::endl;
      if (auto cachedId = SEARCH_TYPE_CACHE(userTypekey)) {
        DbModel::FunDeclTypedefType funDeclTypedefType = {_funcDeclId,
                                                          *cachedId};
        STG.insertClassObj(funDeclTypedefType);
      } else {
        DbModel::FunDeclTypedefType funDeclTypedefType = {_funcDeclId, -1};
        STG.insertClassObj(funDeclTypedefType);
        PendingUpdate update{userTypekey, CacheType::USERTYPE,
                             [_funcDeclId = _funcDeclId](int resolvedId) {
                               DbModel::FunDeclTypedefType updated_record = {
                                   _funcDeclId, resolvedId};
                               STG.insertClassObj(updated_record);
                             }};
        DependencyManager::instance().addDependency(update);
      }
    }
  };

  QualType funcType = decl->getType();
  if (const TypedefType *TT = funcType->getAs<TypedefType>()) {
    processTypedef(TT->getDecl());
  } else if (const ElaboratedType *ET = funcType->getAs<ElaboratedType>()) {
    if (const TypedefType *TT = ET->getNamedType()->getAs<TypedefType>()) {
      processTypedef(TT->getDecl());
    }
  }
}

void FunctionProcessor::recordCoroutine(const FunctionDecl *FD) {
  if (!isCoroutineFunction(FD))
    return;

  ASTContext &Context = FD->getASTContext();
  auto *TraitsTemplate = findCoroutineTraitsTemplate(Context);
  if (!TraitsTemplate)
    return;

  QualType TraitsType = getCoroutineTraitsType(Context, FD, TraitsTemplate);
  if (TraitsType.isNull())
    return;

  KeyType typeKey = KeyGen::Type::makeKey(TraitsType, FD->getASTContext());
  LOG_DEBUG << "Coroutine Trait TypeKey: " << typeKey << std::endl;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey)) {
    DbModel::Coroutine coroutine = {_funcId, *cachedId};
    STG.insertClassObj(coroutine);
  } else {
    DbModel::Coroutine coroutine = {_funcId, -1};
    STG.insertClassObj(coroutine);
    PendingUpdate update{
        typeKey, CacheType::TYPE, [_funcId = _funcId](int resolvedId) {
          DbModel::Coroutine updated_record = {_funcId, resolvedId};
          STG.insertClassObj(updated_record);
        }};
    DependencyManager::instance().addDependency(update);
  }

  FunctionDecl *NewFD = getCoroutineNewFunction(FD, Context);
  if (NewFD) {
    KeyType newFuncKey = KeyGen::Function::makeKey(NewFD, FD->getASTContext());
    LOG_DEBUG << "Function FuncKey: " << newFuncKey << std::endl;
    if (auto cachedId = SEARCH_FUNCTION_CACHE(newFuncKey)) {
      DbModel::CoroutineNew couroutine_new = {_funcId, *cachedId};
      STG.insertClassObj(couroutine_new);
    } else {
      DbModel::CoroutineNew couroutine_new = {_funcId, -1};
      STG.insertClassObj(couroutine_new);
      PendingUpdate update{
          newFuncKey, CacheType::FUNCTION, [_funcId = _funcId](int resolvedId) {
            DbModel::CoroutineNew updated_record = {_funcId, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  FunctionDecl *DelFD = getCoroutineDeleteFunction(FD, Context);
  if (DelFD) {
    KeyType delFuncKey = KeyGen::Function::makeKey(DelFD, FD->getASTContext());
    LOG_DEBUG << "Function FuncKey: " << delFuncKey << std::endl;
    if (auto cachedId = SEARCH_FUNCTION_CACHE(delFuncKey)) {
      DbModel::CoroutineDelete couroutine_delete = {_funcId, *cachedId};
      STG.insertClassObj(couroutine_delete);
    } else {
      DbModel::CoroutineDelete couroutine_delete = {_funcId, -1};
      STG.insertClassObj(couroutine_delete);
      PendingUpdate update{
          delFuncKey, CacheType::FUNCTION, [_funcId = _funcId](int resolvedId) {
            DbModel::CoroutineDelete updated_record = {_funcId, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
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
    if (name.find("operator"
                  "") == 0) { // FIXME: maybe official API support?
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
  if (auto cachedId = SEARCH_TYPE_CACHE(key)) {
    DbModel::DeductionGuideForClass deducGuide = {_funcId, *cachedId};
    STG.insertClassObj(deducGuide);
  } else {
    DbModel::DeductionGuideForClass deducGuide = {_funcId, -1};
    STG.insertClassObj(deducGuide);
    PendingUpdate update{key, CacheType::USERTYPE,
                         [_funcId = _funcId](int resolvedId) {
                           DbModel::DeductionGuideForClass updated_record = {
                               _funcId, resolvedId};
                           STG.insertClassObj(updated_record);
                         }};
    DependencyManager::instance().addDependency(update);
  }
}

Stmt *getFirstNonCompoundStmt(clang::Stmt *S) {
  if (!S)
    return nullptr;

  // Log the statement type
  LOG_DEBUG << "getFirstNonCompoundStmt: processing statement of class "
            << S->getStmtClassName() << std::endl;

  // Recursively process the first non-compound statement
  if (auto *CS = dyn_cast<clang::CompoundStmt>(S)) {
    if (CS->body_empty())
      return S;

    // Get the first child statement
    clang::Stmt *firstChild = *CS->body_begin();
    clang::Stmt *result = getFirstNonCompoundStmt(firstChild);

    // If the result is an expression (like CallExpr), return the original
    // CompoundStmt instead
    if (result && isa<clang::Expr>(result)) {
      LOG_DEBUG << "getFirstNonCompoundStmt: first non-compound statement is "
                   "an Expr ("
                << result->getStmtClassName()
                << "), returning original CompoundStmt instead" << std::endl;
      return S; // Return the original CompoundStmt
    }

    return result;
  }

  // Non-compound statement directly returns itself
  return S;
}
