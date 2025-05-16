#include "core/processor/variable_processor.h"
#include "db/storage_facade.h"
#include "model/db/variable.h"
#include "util/id_generator.h"
#include "util/key_generator/type.h"

void VariableProcessor::routerProcess(const VarDecl *VD) {
  if (!VD || VD->isImplicit())
    return;

  int varId = -1;

  // Classify VarDecl
  if (llvm::isa<clang::FieldDecl>(VD) ||
      (VD->getDeclContext()->isRecord() && VD->isCXXClassMember()))
    varId = processMemberVar(VD); // @membervariable
  else if (VD->hasGlobalStorage() && !VD->isStaticLocal() &&
           VD->getDeclContext()->isFileContext())
    varId = processGlobalVar(VD); // @globalvariable
  else
    varId = processLocalScopeVar(VD); // @localscopevariable
}

// Process Local Scope Variable, return id @localscopevariable
int VariableProcessor::processLocalScopeVar(const VarDecl *VD) {
  int associate_id;
  LocalScopeVarType type;

  if (llvm::isa<clang::ParmVarDecl>(VD)) {
    associate_id = processParam(VD); // @params
    type = LocalScopeVarType::PARAMETER;
  } else {
    associate_id = processLocalVar(VD); // @localvariables
    type = LocalScopeVarType::LOCAL_VARIABLE;
  }

  int id;
  DbModel::LocalScopeVar local_scope_var = {
      id = GENID(LocalScopeVar), associate_id, static_cast<int>(type)};
  STG.insertClassObj(local_scope_var);
  return id;
}

// Process Local Variable. return id @localvariables
int VariableProcessor::processLocalVar(const VarDecl *VD) {
  KeyType typeKey = KeyGen::Type::makeKey(VD->getType(), VD->getASTContext());
  LOG_DEBUG << "Local Variable TypeKey: " << typeKey << std::endl;

  int typeId = -1;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    typeId = *cachedId;
  else {
    // TODO: Dependency Manager...
  }
  int id;
  DbModel::LocalVar localVar = {id = GENID(LocalVar), typeId,
                                VD->getNameAsString()};
  STG.insertClassObj(localVar);
  return id;
}

// Process parameter. return id @params
int VariableProcessor::processParam(const VarDecl *VD) { return -1; }

// Process Global Variable, return id @globalvariable
int VariableProcessor::processGlobalVar(const VarDecl *VD) {
  KeyType typeKey = KeyGen::Type::makeKey(VD->getType(), VD->getASTContext());
  LOG_DEBUG << "Global Variable TypeKey: " << typeKey << std::endl;

  int typeId = -1;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    typeId = *cachedId;
  else {
    // TODO: Dependency Manager...
  }
  int id;
  DbModel::GlobalVar globalVar = {id = GENID(GlobalVar), typeId,
                                  VD->getNameAsString()};
  STG.insertClassObj(globalVar);
  return id;
}

// Process Member Variable, return id @membervariable
int VariableProcessor::processMemberVar(const VarDecl *VD) { return -1; }
