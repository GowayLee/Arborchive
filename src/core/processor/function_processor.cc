#include "core/processor/function_processor.h"
#include "db/storage_facade.h"
#include "model/db/function.h"
#include "model/db/stmt.h"
#include "model/db/type.h"
#include "util/id_generator.h"
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>

Stmt *getFirstNonCompoundStmt(clang::Stmt *S);

// Create base function struct and return id
int FunctionProcessor::handleBaseFunc(const FunctionDecl *decl,
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

  DbModel::FunDecl fun_decl = {GENID(FunDecl), _funcId, _typeId, name,
                               _locIdPair->spec_id};

  STG.insertClassObj(function);
  STG.insertClassObj(fun_decl);
  return function.id;
}

void FunctionProcessor::recordEntryPoint(const FunctionDecl *decl) const {
  if (!decl->hasBody()) // Skip if function has no body
    return;
  Stmt *body = decl->getBody();
  Stmt *entryStmt = getFirstNonCompoundStmt(body);
  if (entryStmt == nullptr) // Skip if entry statement is not found
    return;
  std::string stmtKey =
      DbModel::Stmt::makeKey(entryStmt, decl->getASTContext());
  LOG_DEBUG << "Function entry point StmtKey: " << stmtKey << std::endl;
  int stmtId = -1;
  if (auto cachedId = SEARCH_STMT_CACHE(stmtKey))
    stmtId = *cachedId;
  else {
    LOG_DEBUG << "Stmt cache entry not found, push to pending model queue"
              << std::endl;
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
}

void FunctionProcessor::recordReturnType(const FunctionDecl *decl) {
  // Check Type cache for Id for returnType
  std::string typeKey =
      DbModel::Type::makeKey(decl->getReturnType(), decl->getASTContext());
  LOG_DEBUG << "Function TypeKey: " << typeKey << std::endl;
  _typeId = -1;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    _typeId = *cachedId;
  else {
    LOG_DEBUG << "Type cache entry not found, push to pending model queue"
              << std::endl;
    // TODO: Dependency Manager...
  }
  DbModel::FuncRetType func_ret_type = {_funcId, _typeId};
  STG.insertClassObj(func_ret_type);
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
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::BUILDIN_FUNC);
}

void FunctionProcessor::processUserDefinedLiteral(
    const clang::FunctionDecl *decl) {

  int id =
      handleBaseFunc(cast<FunctionDecl>(decl), FuncType::USER_DEFINED_LITERAL);
}

void FunctionProcessor::processOperatorFunc(const clang::FunctionDecl *decl) {

  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::OPERATOR);
}

void FunctionProcessor::processNormalFunc(const clang::FunctionDecl *decl) {

  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::NORM_FUNC);
}

void FunctionProcessor::processCXXConstructor(const CXXConstructorDecl *decl) {
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::CONSTRUCTOR);
}

void FunctionProcessor::processCXXDestructor(const CXXDestructorDecl *decl) {
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::DESTRUCTOR);
}

void FunctionProcessor::processCXXConversion(const CXXConversionDecl *decl) {
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::CONVERSION_FUNC);
}

void FunctionProcessor::processCXXDeductionGuide(
    const CXXDeductionGuideDecl *decl) {
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::DEDUCTION_GUIDE);
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
