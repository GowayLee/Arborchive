#include "core/processor/specifier_processor.h"
#include "db/storage_facade.h"
#include "model/db/specifiers.h"
#include "util/id_generator.h"
#include "util/logger/macros.h"
#include <clang/AST/DeclCXX.h>

int SpecifierProcessor::getOrCreateSpecifier(const std::string &str) {
  // TODO: Implement specifier cache lookup
  // For now, generate a new ID each time
  GENID(Specifier);
  DbModel::Specifier specifier = {IDGenerator::getLastGeneratedId<DbModel::Specifier>(), str};
  STG.insertClassObj(specifier);
  return specifier.id;
}

void SpecifierProcessor::processTypeQualifiers(int type_id, QualType qualType) {
  if (qualType.isConstQualified()) {
    int spec_id = getOrCreateSpecifier("const");
    insertTypeSpecifiers(type_id, spec_id);
  }

  if (qualType.isVolatileQualified()) {
    int spec_id = getOrCreateSpecifier("volatile");
    insertTypeSpecifiers(type_id, spec_id);
  }

  if (qualType.isRestrictQualified()) {
    int spec_id = getOrCreateSpecifier("restrict");
    insertTypeSpecifiers(type_id, spec_id);
  }
}

void SpecifierProcessor::processFunctionSpecifiers(int func_id, const FunctionDecl *FD) {
  // static (仅成员函数)
  if (FD->isStatic()) {
    int spec_id = getOrCreateSpecifier("static");
    insertFunSpecifiers(func_id, spec_id);
  }

  // virtual
  if (FD->isVirtualAsWritten()) {
    int spec_id = getOrCreateSpecifier("virtual");
    insertFunSpecifiers(func_id, spec_id);
  }

  // inline
  if (FD->isInlineSpecified()) {
    int spec_id = getOrCreateSpecifier("inline");
    insertFunSpecifiers(func_id, spec_id);
  }

  // constexpr
  if (FD->isConstexprSpecified()) {
    int spec_id = getOrCreateSpecifier("constexpr");
    insertFunSpecifiers(func_id, spec_id);
  }

  // pure virtual (= 0)
  if (FD->isPureVirtual()) {
    int spec_id = getOrCreateSpecifier("pure_virtual");
    insertFunSpecifiers(func_id, spec_id);
  }

  // 访问说明符
  if (const auto *MD = llvm::dyn_cast<const CXXMethodDecl>(FD)) {
    switch (MD->getAccess()) {
    case AS_public:
      insertFunSpecifiers(func_id, getOrCreateSpecifier("public"));
      break;
    case AS_protected:
      insertFunSpecifiers(func_id, getOrCreateSpecifier("protected"));
      break;
    case AS_private:
      insertFunSpecifiers(func_id, getOrCreateSpecifier("private"));
      break;
    default:
      break;
    }
  }
}

void SpecifierProcessor::processVariableSpecifiers(int var_id, const VarDecl *VD) {
  // 处理存储类说明符（storage class specifiers）
  StorageClass storageClass = VD->getStorageClass();
  if (storageClass != SC_None) {
    std::string storageClassStr;
    switch (storageClass) {
    case SC_Static:
      storageClassStr = "static";
      break;
    case SC_Extern:
      storageClassStr = "extern";
      break;
    case SC_PrivateExtern:
      storageClassStr = "private_extern";
      break;
    case SC_Auto:
      storageClassStr = "auto";
      break;
    case SC_Register:
      storageClassStr = "register";
      break;
    default:
      storageClassStr = "unknown";
    }
    if (storageClassStr != "unknown") {
      insertVarSpecifiers(var_id, getOrCreateSpecifier(storageClassStr));
    }
  }

  // 处理类型限定符（type qualifiers）
  QualType qualType = VD->getType();

  if (qualType.isConstQualified()) {
    insertVarSpecifiers(var_id, getOrCreateSpecifier("const"));
  }

  if (qualType.isVolatileQualified()) {
    insertVarSpecifiers(var_id, getOrCreateSpecifier("volatile"));
  }

  if (qualType.isRestrictQualified()) {
    insertVarSpecifiers(var_id, getOrCreateSpecifier("restrict"));
  }

  // 线程局部存储
  if (VD->getTLSKind() != VarDecl::TLS_None) {
    insertVarSpecifiers(var_id, getOrCreateSpecifier("thread_local"));
  }

  // 内联变量（C++17）
  if (VD->isInline()) {
    insertVarSpecifiers(var_id, getOrCreateSpecifier("inline"));
  }

  // constexpr 变量
  if (VD->isConstexpr()) {
    insertVarSpecifiers(var_id, getOrCreateSpecifier("constexpr"));
  }

  // 静态成员变量
  if (VD->isStaticDataMember()) {
    insertVarSpecifiers(var_id, getOrCreateSpecifier("static_member"));
  }

  // 可见性属性
  switch (VD->getVisibility()) {
  case HiddenVisibility:
    insertVarSpecifiers(var_id, getOrCreateSpecifier("visibility_hidden"));
    break;
  case ProtectedVisibility:
    insertVarSpecifiers(var_id, getOrCreateSpecifier("visibility_protected"));
    break;
  default:
    break;
  }
}

void SpecifierProcessor::insertTypeSpecifiers(int type_id, int spec_id) {
  DbModel::TypeSpecifiers ts = {type_id, spec_id};
  STG.insertClassObj(ts);
}

void SpecifierProcessor::insertFunSpecifiers(int func_id, int spec_id) {
  DbModel::FunSpecifiers fs = {func_id, spec_id};
  STG.insertClassObj(fs);
}

void SpecifierProcessor::insertVarSpecifiers(int var_id, int spec_id) {
  DbModel::VarSpecifiers vs = {var_id, spec_id};
  STG.insertClassObj(vs);
}
