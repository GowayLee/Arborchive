#ifndef _COROUTINE_HELPER_H_
#define _COROUTINE_HELPER_H_

#include "util/logger/macros.h"
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/ExceptionSpecificationType.h>
#include <clang/Basic/OperatorKinds.h>

using namespace clang;

bool isCoroutineFunction(const FunctionDecl *FD) {
  if (!FD->hasBody() || !FD->isThisDeclarationADefinition())
    return false;

  // 检查函数体中是否包含协程关键字
  Stmt *Body = FD->getBody();

  // 方法1：检查函数体是否是CoroutineBodyStmt
  if (llvm::isa<CoroutineBodyStmt>(Body))
    return true;

  // 方法2：遍历AST查找协程关键字表达式/语句
  class CoroutineKeywordFinder
      : public RecursiveASTVisitor<CoroutineKeywordFinder> {
  public:
    bool HasCoroutineKeyword = false;

    bool VisitCoawaitExpr(CoawaitExpr *) {
      HasCoroutineKeyword = true;
      return false; // 找到一个就停止遍历
    }

    bool VisitCoyieldExpr(CoyieldExpr *) {
      HasCoroutineKeyword = true;
      return false;
    }

    bool VisitCoreturnStmt(CoreturnStmt *) {
      HasCoroutineKeyword = true;
      return false;
    }
  };

  CoroutineKeywordFinder Finder;
  Finder.TraverseStmt(Body);
  return Finder.HasCoroutineKeyword;
}

ClassTemplateDecl *findCoroutineTraitsTemplate(ASTContext &Context) {
  // 查找std命名空间
  NamespaceDecl *StdNamespace = nullptr;
  auto StdIdentifier = &Context.Idents.get("std");
  auto Result = Context.getTranslationUnitDecl()->lookup(StdIdentifier);
  for (auto *Decl : Result)
    if (auto *NamespaceDecl = llvm::dyn_cast<clang::NamespaceDecl>(Decl)) {
      StdNamespace = NamespaceDecl;
      break;
    }

  if (!StdNamespace)
    return nullptr;

  // 尝试在std命名空间中查找coroutine_traits (C++20)
  auto TraitsIdentifier = &Context.Idents.get("coroutine_traits");
  auto TraitsResult = StdNamespace->lookup(TraitsIdentifier);
  for (auto *Decl : TraitsResult)
    if (auto *TmplDecl = llvm::dyn_cast<ClassTemplateDecl>(Decl))
      return TmplDecl;

  // 查找std::experimental命名空间
  NamespaceDecl *StdExpNamespace = nullptr;
  auto ExpIdentifier = &Context.Idents.get("experimental");
  auto ExpResult = StdNamespace->lookup(ExpIdentifier);
  for (auto *Decl : ExpResult)
    if (auto *NamespaceDecl = llvm::dyn_cast<clang::NamespaceDecl>(Decl)) {
      StdExpNamespace = NamespaceDecl;
      break;
    }

  if (!StdExpNamespace)
    return nullptr;

  // 在std::experimental中查找coroutine_traits
  TraitsResult = StdExpNamespace->lookup(TraitsIdentifier);
  for (auto *Decl : TraitsResult)
    if (auto *TmplDecl = llvm::dyn_cast<ClassTemplateDecl>(Decl))
      return TmplDecl;

  // 尝试查找std::experimental::resumable_traits (早期实验版本)
  auto ResumableIdentifier = &Context.Idents.get("resumable_traits");
  auto ResumableResult = StdExpNamespace->lookup(ResumableIdentifier);
  for (auto *Decl : ResumableResult)
    if (auto *TmplDecl = llvm::dyn_cast<ClassTemplateDecl>(Decl))
      return TmplDecl;

  return nullptr;
}

QualType getCoroutineTraitsType(ASTContext &Context, const FunctionDecl *FD,
                                ClassTemplateDecl *TraitsTemplate) {
  if (!TraitsTemplate)
    return QualType();

  // 构建模板参数列表
  llvm::SmallVector<TemplateArgument, 8> TemplateArgs;

  // 1. 返回类型
  QualType ReturnType = FD->getReturnType();
  TemplateArgs.push_back(TemplateArgument(ReturnType));

  // 2. 参数类型
  for (const auto *Param : FD->parameters()) {
    QualType ParamType = Param->getType();
    TemplateArgs.push_back(TemplateArgument(ParamType));
  }

  // 3. this指针类型
  if (auto *Method = llvm::dyn_cast<CXXMethodDecl>(FD)) {
    if (!Method->isStatic()) {
      QualType ThisType = Method->getThisType();
      TemplateArgs.push_back(TemplateArgument(ThisType));
    }
  }

  // 创建模板名称
  TemplateName TemplateName(TraitsTemplate);

  // 创建模板特化类型
  QualType TraitsType = Context.getTemplateSpecializationType(
      TemplateName, TemplateArgs, QualType());

  return TraitsType;
}

FunctionDecl *getCoroutineNewFunction(const FunctionDecl *FD,
                                      ASTContext &Context) {
  if (!FD->hasBody())
    return nullptr;

  Stmt *Body = FD->getBody();
  auto *CBS = llvm::dyn_cast<CoroutineBodyStmt>(Body);
  if (!CBS)
    return nullptr;

  // 获取 operator new 的调用表达式
  if (auto *Allocator = CBS->getAllocate())
    if (auto *Call = llvm::dyn_cast<CallExpr>(Allocator))
      // 获取被调用的函数
      if (auto *CalledFn = Call->getDirectCallee())
        // 检查是否是 operator new
        if (CalledFn->isOverloadedOperator() &&
            CalledFn->getOverloadedOperator() == OO_New)
          return CalledFn;

  return nullptr;
}

FunctionDecl *getCoroutineDeleteFunction(const FunctionDecl *FD,
                                         ASTContext &Context) {
  if (!FD->hasBody())
    return nullptr;

  Stmt *Body = FD->getBody();
  auto *CBS = llvm::dyn_cast<CoroutineBodyStmt>(Body);
  if (!CBS)
    return nullptr;

  // 获取 operator new 的调用表达式
  if (auto *Allocator = CBS->getAllocate())
    if (auto *Call = llvm::dyn_cast<CallExpr>(Allocator))
      // 获取被调用的函数
      if (auto *CalledFn = Call->getDirectCallee())
        // 检查是否是 operator new
        if (CalledFn->isOverloadedOperator() &&
            CalledFn->getOverloadedOperator() == OO_Delete)
          return CalledFn;

  return nullptr;
}

#endif // _COROUTINE_HELPER_H_
