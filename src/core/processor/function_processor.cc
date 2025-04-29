#include "core/processor/function_processor.h"
#include "db/storage_facade.h"
#include "model/db/function.h"
#include "util/id_generator.h"
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/LLVM.h>

using namespace clang;

// Create base function struct and return id
int FunctionProcessor::handleFunc(FunctionDecl *decl, FuncType type) {
  DbModel::Function function = {GENID(Function), decl->getNameAsString(),
                                static_cast<int>(type)};
  STG.insertClassObj(function);
  return function.id;
}

void FunctionProcessor::processCXXConstructor(CXXConstructorDecl *decl) {
  if (!decl)
    return;

  int id = handleFunc(cast<FunctionDecl>(decl), FuncType::CONSTRUCTOR);
}
void FunctionProcessor::processCXXDestructor(CXXDestructorDecl *decl) {
  if (!decl)
    return;

  int id = handleFunc(cast<FunctionDecl>(decl), FuncType::DESTRUCTOR);
}
void FunctionProcessor::processCXXConversion(CXXConversionDecl *decl) {
  if (!decl)
    return;

  int id = handleFunc(cast<FunctionDecl>(decl), FuncType::CONVERSION_FUNC);
}

void FunctionProcessor::processCXXDeductionGuide(CXXDeductionGuideDecl *decl) {
  if (!decl)
    return;

  int id = handleFunc(cast<FunctionDecl>(decl), FuncType::DEDUCTION_GUIDE);
}
