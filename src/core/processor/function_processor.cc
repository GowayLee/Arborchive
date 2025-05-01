#include "core/processor/function_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/function.h"
#include "model/db/type.h"
#include "util/id_generator.h"
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>

// Create base function struct and return id
int FunctionProcessor::handleBaseFunc(const FunctionDecl *decl,
                                      const FuncType type) const {
  LocIdPair *locIdPair = PROC_DEFT(cast<Decl>(decl), ast_context_);
  std::string name = decl->getNameAsString();
  DbModel::Function function = {GENID(Function), name, static_cast<int>(type)};

  // Check Type cache for Id for returnType
  std::string typeKey =
      DbModel::Type::makeKey(decl->getReturnType(), decl->getASTContext());
  LOG_DEBUG << "Function TypeKey: " << typeKey << std::endl;
  int typeId = -1;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    typeId = *cachedId;
  else {
    LOG_DEBUG << "Type cache entry not found, push to pending model queue"
              << std::endl;
    // TODO: Dependency Manager...
  }

  DbModel::FunDecl fun_decl = {GENID(FunDecl), function.id, typeId, name,
                               locIdPair->spec_id};
  DbModel::FuncRetType func_ret_type = {function.id, typeId};

  STG.insertClassObj(function);
  STG.insertClassObj(fun_decl);
  STG.insertClassObj(func_ret_type);
  return function.id;
}

// Router to process functions of @operator @builtin_function
// @user_defined_function, @normal_function
void FunctionProcessor::routerProcess(const clang::FunctionDecl *decl) const {
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
    if (name.starts_with("__builtin__")) {
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

void FunctionProcessor::processBuiltinFunc(
    const clang::FunctionDecl *decl) const {
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::BUILDIN_FUNC);
}

void FunctionProcessor::processUserDefinedLiteral(
    const clang::FunctionDecl *decl) const {

  int id =
      handleBaseFunc(cast<FunctionDecl>(decl), FuncType::USER_DEFINED_LITERAL);
}

void FunctionProcessor::processOperatorFunc(
    const clang::FunctionDecl *decl) const {

  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::OPERATOR);
}

void FunctionProcessor::processNormalFunc(
    const clang::FunctionDecl *decl) const {

  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::NORM_FUNC);
}

void FunctionProcessor::processCXXConstructor(
    const CXXConstructorDecl *decl) const {
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::CONSTRUCTOR);
}

void FunctionProcessor::processCXXDestructor(
    const CXXDestructorDecl *decl) const {
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::DESTRUCTOR);
}

void FunctionProcessor::processCXXConversion(
    const CXXConversionDecl *decl) const {
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::CONVERSION_FUNC);
}

void FunctionProcessor::processCXXDeductionGuide(
    const CXXDeductionGuideDecl *decl) const {
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::DEDUCTION_GUIDE);
}
