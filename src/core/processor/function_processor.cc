#include "core/processor/function_processor.h"
#include "db/storage_facade.h"
#include "model/db/function.h"
#include "util/id_generator.h"
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>

using namespace clang;

// Create base function struct and return id
int FunctionProcessor::handleBaseFunc(const FunctionDecl *decl,
                                      const FuncType type) const {
  DbModel::Function function = {GENID(Function), decl->getNameAsString(),
                                static_cast<int>(type)};
  STG.insertClassObj(function);
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
  llvm::StringRef name = decl->getName();
  if (decl->getBuiltinID() != 0 || name.starts_with("__builtin__")) {
    processBuiltinFunc(cast<FunctionDecl>(decl));
    return;
  }

  // Determine isUserDefinedLiteral
  if (name.find("operator\"\"") == 0) { // FIXME: maybe official API support?
    processUserDefinedLiteral(cast<FunctionDecl>(decl));
    return;
  }

  // Otherwise, isNormalFunction
  processUserDefinedLiteral(cast<FunctionDecl>(decl));
}

void FunctionProcessor::processBuiltinFunc(
    const clang::FunctionDecl *decl) const {
  int id = handleBaseFunc(cast<FunctionDecl>(decl), FuncType::BUILDIN_FUNC);
}

void FunctionProcessor::processUserDefinedLiteral(
    const clang::FunctionDecl *decl) const {}

void FunctionProcessor::processOperatorFunc(
    const clang::FunctionDecl *decl) const {}

void FunctionProcessor::processNormalFunc(
    const clang::FunctionDecl *decl) const {}

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
