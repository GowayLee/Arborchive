#include "core/processor/variable_processor.h"
#include "core/srcloc_recorder.h"
#include "db/dependency_manager.h"
#include "db/storage_facade.h"
#include "model/db/variable.h"
#include "util/id_generator.h"
#include "util/key_generator/element.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/type.h"
#include "util/key_generator/variable.h"
#include "util/logger/macros.h"
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/LLVM.h>

int VariableProcessor::processVarDecl(const VarDecl *VD) {
  if (!VD || VD->isImplicit())
    return -1;

  int varId;

  // Classify VarDecl first to get the specific variable ID
  // REFACTORED: Use direct entity IDs instead of intermediary variable IDs
  if (llvm::isa<clang::FieldDecl>(VD) ||
      (VD->getDeclContext()->isRecord() && VD->isCXXClassMember())) {
    varId = processMemberVar(VD); // @membervariable
  } else if (VD->hasGlobalStorage() && !VD->isStaticLocal() &&
             VD->getDeclContext()->isFileContext()) {
    varId = processGlobalVar(VD); // @globalvariable
  } else {
    varId = processLocalScopeVar(VD); // @localvariables or @params directly
  }

  LocIdPair *locIdPair = SrcLocRecorder::processDefault(VD, ast_context_);
  _name = VD->getNameAsString();
  _varDeclId = GENID(VarDecl);

  // Handle Type Dependency
  KeyType typeKey = KeyGen::Type::makeKey(VD->getType(), ast_context_);
  LOG_DEBUG << "Variable TypeKey: " << typeKey << std::endl;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey)) {
    _typeId = *cachedId;
  } else {
    _typeId = -1;
    PendingUpdate update{typeKey, CacheType::TYPE,
                         [_varDeclId = _varDeclId, varId = varId, name = _name,
                          spec_id = locIdPair->spec_id](int resolvedId) {
                           DbModel::VarDecl updated_record = {
                               _varDeclId, varId, resolvedId, name, spec_id};
                           STG.insertClassObj(updated_record);
                         }};
    DependencyManager::instance().addDependency(update);
  }

  DbModel::VarDecl varDecl = {_varDeclId, varId, _typeId, _name,
                              locIdPair->spec_id};

  // Maintain cache
  KeyType VDKey = KeyGen::Var::makeKey(VD, ast_context_);
  INSERT_VARIABLE_CACHE(VDKey, varId);

  if (VD->isThisDeclarationADefinition()) {
    DbModel::VarDef varDef = {_varDeclId};
    STG.insertClassObj(varDef);
  }
  recordSpecialize(VD);
  recordStructuredBinding(VD);

  STG.insertClassObj(varDecl);
  return _varDeclId;
}

void VariableProcessor::recordSpecialize(const VarDecl *VD) {
  if (auto *specialized =
          clang::dyn_cast<clang::VarTemplateSpecializationDecl>(VD)) {
    DbModel::VarSpecialized varSpecialized = {_varDeclId};
    STG.insertClassObj(varSpecialized);
  }
}

void VariableProcessor::recordStructuredBinding(const VarDecl *VD) {
  if (auto *bindingDecl = clang::dyn_cast<clang::DecompositionDecl>(VD)) {
    DbModel::IsStructuredBinding isStructuredBinding = {_varId};
    STG.insertClassObj(isStructuredBinding);
  }
}

void VariableProcessor::recordRequire(const VarDecl *VD) {
  // 获取约束表达式
  const clang::Expr *CE = VD->getTrailingRequiresClause().ConstraintExpr;
  if (!CE)
    return;
  KeyType exprKey = KeyGen::Expr_::makeKey(CE, ast_context_);
  LOG_DEBUG << "Variable require Expr key: " << exprKey << std::endl;
  if (auto cachedId = SEARCH_EXPR_CACHE(exprKey)) {
    DbModel::VarRequire varRequire = {_varDeclId, *cachedId};
    STG.insertClassObj(varRequire);
  } else {
    DbModel::VarRequire varRequire = {_varDeclId, -1};
    STG.insertClassObj(varRequire);
    PendingUpdate update{
        exprKey, CacheType::EXPR, [_varDeclId = _varDeclId](int resolvedId) {
          DbModel::VarRequire updated_record = {_varDeclId, resolvedId};
          STG.insertClassObj(updated_record);
        }};
    DependencyManager::instance().addDependency(update);
  }
}

// Process Local Scope Variable, return direct entity ID
// REFACTORED: Return direct entity IDs instead of LocalScopeVar intermediary
int VariableProcessor::processLocalScopeVar(const VarDecl *VD) {
  if (llvm::isa<clang::ParmVarDecl>(VD)) { // FIXME: No function context
    return processParam(VD);               // @params
  } else {
    return processLocalVar(VD); // @localvariables
  }
}

// Process Local Variable. return id @localvariables
int VariableProcessor::processLocalVar(const VarDecl *VD) {
  DbModel::LocalVar localVar = {GENID(LocalVar), _typeId,
                                VD->getNameAsString()};
  STG.insertClassObj(localVar);
  return localVar.id;
}

// Process parameter. return id @params
int VariableProcessor::processParam(const VarDecl *VD) {
  const FunctionDecl *FD = dyn_cast<FunctionDecl>(VD->getDeclContext());

  if (!FD) {
    LOG_WARNING << "Parameter '" << VD->getNameAsString()
                << "' has no function context." << std::endl;
    return -1;
  }

  // Get parameter index
  size_t index;
  auto *PVD = llvm::cast<clang::ParmVarDecl>(VD);
  // Get the parameter list
  auto Params = FD->parameters();
  // Iterate through parameters to find the index
  for (index = 0; index < Params.size(); ++index)
    if (Params[index] == PVD) {
      LOG_DEBUG << "Parameter '" << PVD->getNameAsString() << "' in function '"
                << FD->getNameAsString() << "' has index: " << index
                << std::endl;
      break;
    }

  // Extract type information of the param and insert into cache
  const QualType QT = PVD->getOriginalType();
  KeyType paramTypeKey = KeyGen::Type::makeKey(QT, ast_context_);
  // TODO: get type id info

  // Get parameterized element
  KeyType elementKey = KeyGen::Element::makeKey(FD, ast_context_);
  int elementId = -1;
  if (auto cachedId = SEARCH_ELEMENT_CACHE(elementKey)) {
    elementId = *cachedId;
  } else {
    PendingUpdate update{elementKey, CacheType::ELEMENT,
                         [paramId = GENID(Parameter), index = index,
                          typeId = _typeId](int resolvedId) {
                           DbModel::Parameter updated_record = {
                               paramId, resolvedId, static_cast<int>(index),
                               typeId};
                           STG.insertClassObj(updated_record);
                         }};
    DependencyManager::instance().addDependency(update);
  }

  DbModel::Parameter param = {GENID(Parameter), elementId,
                              static_cast<int>(index), _typeId};
  STG.insertClassObj(param);
  return param.id;
}

// Process Global Variable, return id @globalvariable
int VariableProcessor::processGlobalVar(const VarDecl *VD) {
  DbModel::GlobalVar globalVar = {GENID(GlobalVar), _typeId,
                                  VD->getNameAsString()};
  STG.insertClassObj(globalVar);
  return globalVar.id;
}

// Process Member Variable, return id @membervariable
int VariableProcessor::processMemberVar(const VarDecl *VD) {
  DbModel::MemberVar memberVar = {GENID(MemberVar), _typeId, _name};
  STG.insertClassObj(memberVar);
  return memberVar.id;
}

int VariableProcessor::processParmVarDecl(const ParmVarDecl *PVD) {
  if (!PVD || PVD->isImplicit())
    return -1;

  // Process parameter and get var ID
  int varId = processParam(PVD);

  // Generate var_decl_id and create VarDecl record
  LocIdPair *locIdPair = SrcLocRecorder::processDefault(PVD, ast_context_);
  _name = PVD->getNameAsString();
  _varDeclId = GENID(VarDecl);

  // Handle Type Dependency
  KeyType typeKey = KeyGen::Type::makeKey(PVD->getType(), ast_context_);
  LOG_DEBUG << "Parameter TypeKey: " << typeKey << std::endl;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey)) {
    _typeId = *cachedId;
  } else {
    _typeId = -1;
    PendingUpdate update{typeKey, CacheType::TYPE,
                         [_varDeclId = _varDeclId, varId = varId, name = _name,
                          spec_id = locIdPair->spec_id](int resolvedId) {
                           DbModel::VarDecl updated_record = {
                               _varDeclId, varId, resolvedId, name, spec_id};
                           STG.insertClassObj(updated_record);
                         }};
    DependencyManager::instance().addDependency(update);
  }

  DbModel::VarDecl varDecl = {_varDeclId, varId, _typeId, _name,
                              locIdPair->spec_id};
  STG.insertClassObj(varDecl);
  return _varDeclId;
}

int VariableProcessor::processFieldDecl(const FieldDecl *FD) {
  if (!FD)
    return -1;

  // Process member variable and get var ID
  LocIdPair *locIdPair = SrcLocRecorder::processDefault(FD, ast_context_);
  _name = FD->getNameAsString();
  _varDeclId = GENID(VarDecl);

  // Handle Type Dependency - _typeId will be set by ASTVisitor before calling
  // this
  KeyType typeKey = KeyGen::Type::makeKey(FD->getType(), ast_context_);
  LOG_DEBUG << "Field TypeKey: " << typeKey << std::endl;
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey)) {
    _typeId = *cachedId;
  } else {
    _typeId = -1;
    // Note: For fields, we don't create a VarDecl record with dependency
    // because the type will be processed by ASTVisitor
  }

  // Process member variable to get varId
  int varId = processMemberVar(FD);

  DbModel::VarDecl varDecl = {_varDeclId, varId, _typeId, _name,
                              locIdPair->spec_id};
  STG.insertClassObj(varDecl);
  return _varDeclId;
}

int VariableProcessor::processMemberVar(const FieldDecl *FD) {
  // For fields, _typeId should be set by the caller before calling this method
  _name = FD->getNameAsString();

  // 使用缓存机制：先检查是否已存在
  KeyType fieldKey = KeyGen::Var::makeKey(FD, ast_context_);

  if (auto cachedId = SEARCH_MEMBERVAR_CACHE(fieldKey)) {
    LOG_DEBUG << "MemberVar '" << _name << "' found in cache with ID: "
              << *cachedId << std::endl;
    return *cachedId;
  }

  // 缓存未命中，创建新记录
  DbModel::MemberVar memberVar = {GENID(MemberVar), _typeId, _name};
  STG.insertClassObj(memberVar);

  // 插入缓存
  INSERT_MEMBERVAR_CACHE(fieldKey, memberVar.id);
  LOG_DEBUG << "Created and cached MemberVar '" << _name
            << "' with key: " << fieldKey << " -> ID: " << memberVar.id << std::endl;

  return memberVar.id;
}
