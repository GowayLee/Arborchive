#include "core/processor/variable_processor.h"
#include "core/srcloc_recorder.h"
#include "db/dependency_manager.h"
#include "db/storage_facade.h"
#include "model/db/class.h"
#include "model/db/declaration.h"
#include "model/db/variable.h"
#include "util/id_generator.h"
#include "util/key_generator/element.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/type.h"
#include "util/key_generator/variable.h"
#include "util/logger/macros.h"
#include <clang/AST/Decl.h>
#include <clang/Basic/LLVM.h>

void VariableProcessor::routerProcess(const VarDecl *VD) {
  if (!VD || VD->isImplicit())
    return;

  int varId;
  VarType varType;

  // Classify VarDecl first to get the specific variable ID
  if (llvm::isa<clang::FieldDecl>(VD) ||
      (VD->getDeclContext()->isRecord() && VD->isCXXClassMember())) {
    varId = processMemberVar(VD); // @membervariable
    varType = VarType::MEMBER_VARIABLE;
  } else if (VD->hasGlobalStorage() && !VD->isStaticLocal() &&
             VD->getDeclContext()->isFileContext()) {
    varId = processGlobalVar(VD); // @globalvariable
    varType = VarType::GLOBAL_VARIABLE;
  } else {
    varId = processLocalScopeVar(VD); // @localscopevariable
    varType = VarType::LOCAL_SCOPE_VARIABLE;
  }

  DbModel::Variable variable = {_varId = GENID(Variable), varId,
                                static_cast<int>(varType)};

  LocIdPair *locIdPair =
      SrcLocRecorder::processDefault(VD, &VD->getASTContext());
  _name = VD->getNameAsString();
  _varDeclId = GENID(VarDecl);

  // Handle Type Dependency
  KeyType typeKey = KeyGen::Type::makeKey(VD->getType(), VD->getASTContext());
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
  KeyType VDKey = KeyGen::Var::makeKey(VD, VD->getASTContext());
  INSERT_VARIABLE_CACHE(VDKey, varDecl.id);

  DbModel::Declaration declaration = {GENID(Declaration), _varDeclId,
                                      static_cast<int>(DeclType::VARIABLE)};

  if (VD->isThisDeclarationADefinition()) {
    DbModel::VarDef varDef = {_varDeclId};
    STG.insertClassObj(varDef);
  }
  recordSpecialize(VD);
  recordSpecifier(VD);
  recordStructuredBinding(VD);

  STG.insertClassObj(declaration);
  STG.insertClassObj(variable);
  STG.insertClassObj(varDecl);
}

void VariableProcessor::recordSpecialize(const VarDecl *VD) {
  if (auto *specialized =
          clang::dyn_cast<clang::VarTemplateSpecializationDecl>(VD)) {
    DbModel::VarSpecialized varSpecialized = {_varDeclId};
    STG.insertClassObj(varSpecialized);
  }
}

void VariableProcessor::recordSpecifier(const VarDecl *VD) {
  // 处理存储类说明符（storage class specifiers）
  clang::StorageClass storageClass = VD->getStorageClass();
  if (storageClass != clang::SC_None) {
    std::string storageClassStr;
    switch (storageClass) {
    case clang::SC_Static:
      storageClassStr = "static";
      break;
    case clang::SC_Extern:
      storageClassStr = "extern";
      break;
    case clang::SC_PrivateExtern:
      storageClassStr = "private_extern";
      break;
    case clang::SC_Auto:
      storageClassStr = "auto";
      break;
    case clang::SC_Register:
      storageClassStr = "register";
      break;
    default:
      storageClassStr = "unknown";
    }
    if (storageClassStr != "unknown") {
      DbModel::VarDeclSpec varDeclSpec = {_varDeclId, storageClassStr};
      STG.insertClassObj(varDeclSpec);
    }
  }

  // 处理类型限定符（type qualifiers）
  clang::QualType qualType = VD->getType();

  if (qualType.isConstQualified()) {
    DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "const"};
    STG.insertClassObj(varDeclSpec);
  }

  if (qualType.isVolatileQualified()) {
    DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "volatile"};
    STG.insertClassObj(varDeclSpec);
  }

  if (qualType.isRestrictQualified()) {
    DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "restrict"};
    STG.insertClassObj(varDeclSpec);
  }

  // 检查是否为线程局部存储
  if (VD->getTLSKind() != clang::VarDecl::TLS_None) {
    DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "thread_local"};
    STG.insertClassObj(varDeclSpec);
  }

  // 检查是否为内联变量（C++17特性）
  if (VD->isInline()) {
    DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "inline"};
    STG.insertClassObj(varDeclSpec);
  }

  // 检查是否为constexpr变量
  if (VD->isConstexpr()) {
    DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "constexpr"};
    STG.insertClassObj(varDeclSpec);
  }

  // 检查是否为静态成员变量
  if (VD->isStaticDataMember()) {
    DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "static_member"};
    STG.insertClassObj(varDeclSpec);
  }

  // 检查可见性属性
  switch (VD->getVisibility()) {
  case clang::DefaultVisibility:
    break;
  case clang::HiddenVisibility: {
    DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "visibility_hidden"};
    STG.insertClassObj(varDeclSpec);
    break;
  }
  case clang::ProtectedVisibility: {
    DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "visibility_protected"};
    STG.insertClassObj(varDeclSpec);
    break;
  }
  }

  // // 检查对齐方式
  // if (VD->hasAttr<clang::AlignedAttr>()) {
  //   DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "aligned"};
  //   STG.insertClassObj(varDeclSpec);
  // }

  // // 检查其他常见属性
  // if (VD->hasAttr<clang::DeprecatedAttr>()) {
  //   DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "deprecated"};
  //   STG.insertClassObj(varDeclSpec);
  // }
  // if (VD->hasAttr<clang::UnusedAttr>()) {
  //   DbModel::VarDeclSpec varDeclSpec = {_varDeclId, "unused"};
  //   STG.insertClassObj(varDeclSpec);
  // }
}

void VariableProcessor::recordStructuredBinding(const VarDecl *VD) {
  if (auto *bindingDecl = clang::dyn_cast<clang::DecompositionDecl>(VD)) {
    DbModel::IsStructuredBinding isStructuredBinding = {_varId};
    STG.insertClassObj(isStructuredBinding);
  }
}

void VariableProcessor::recordRequire(const VarDecl *VD) {
  // 获取约束表达式
  const clang::Expr *CE = VD->getTrailingRequiresClause();
  if (!CE)
    return;
  KeyType exprKey = KeyGen::Expr_::makeKey(CE, VD->getASTContext());
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

// Process Local Scope Variable, return id @localscopevariable
int VariableProcessor::processLocalScopeVar(const VarDecl *VD) {
  int associate_id;
  LocalScopeVarType type;

  if (llvm::isa<clang::ParmVarDecl>(VD)) { // FIXME: No function context
    associate_id = processParam(VD);       // @params
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

  // Get parameterized element
  KeyType elementKey = KeyGen::Element::makeKey(FD, FD->getASTContext());
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
  DbModel::Member member = {GENID(Member), memberVar.id,
                            static_cast<int>(MemberType::MEMBERVARIABLE)};
  STG.insertClassObj(memberVar);
  STG.insertClassObj(member);
  return memberVar.id;
}
