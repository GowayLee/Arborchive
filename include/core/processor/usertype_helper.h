#ifndef _USERTYPE_HELPER_H_
#define _USERTYPE_HELPER_H_

#include "db/storage_facade.h"
#include "model/db/type.h"
#include "util/logger/macros.h"
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Type.h>

using namespace clang;

void record_is_pod_class(const Type *TP, const ASTContext &ast_context,
                         int usertype_id) {
  if (!TP) {
    LOG_WARNING << "Null type pointer" << std::endl;
    return;
  }

  // 确保是用户定义的类型
  const CXXRecordDecl *recordDecl = TP->getAsCXXRecordDecl();
  if (!recordDecl) {
    LOG_WARNING << "Type is not a C++ record" << std::endl;
    return;
  }

  // 检查是否为POD类型
  bool isPOD = recordDecl->isPOD();
  // 可以记录更详细的信息
  LOG_DEBUG << "Type " << recordDecl->getNameAsString()
            << ": isPodClass=" << isPOD << std::endl;

  if (!isPOD)
    return;

  DbModel::IsPodClass isPodClassModel = {usertype_id};
  STG.insertClassObj(isPodClassModel);
}

void record_is_standard_layout_class(const Type *TP,
                                     const ASTContext &ast_context,
                                     int userTypeId) {
  if (!TP) {
    LOG_WARNING << "Null type pointer" << std::endl;
    return;
  }
  // 获取C++记录声明
  const CXXRecordDecl *recordDecl = TP->getAsCXXRecordDecl();
  if (!recordDecl) {
    LOG_WARNING << "Type is not a C++ record" << std::endl;
    return;
  }
  // 检查标准布局属性
  bool isStandardLayout = recordDecl->isStandardLayout();
  // 可以记录更详细的信息
  LOG_DEBUG << "Type " << recordDecl->getNameAsString()
            << ": isStandardLayout=" << isStandardLayout << std::endl;

  if (!isStandardLayout)
    return;

  DbModel::IsStandartLayoutClass isStandardLayoutClassModel = {userTypeId};
  STG.insertClassObj(isStandardLayoutClassModel);
}

void record_is_complete(const Type *TP, const ASTContext &ast_context,
                        int userTypeId) {
  if (!TP) {
    LOG_WARNING << "Null type pointer" << std::endl;
    return;
  }

  // 检查类型是否完整
  bool isComplete = !TP->isIncompleteType();
  // 可以记录更详细的信息
  LOG_DEBUG << "Type " << TP->getTypeClassName()
            << ": isComplete=" << isComplete << std::endl;

  if (!isComplete)
    return;

  DbModel::IsComplete isCompleteModel = {userTypeId};
  STG.insertClassObj(isCompleteModel);
}

#endif // _USERTYPE_HELPER_H_
