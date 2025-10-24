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

const ClassTemplateDecl *findCoroutineTraitsTemplate(ASTContext &ast_context) {
  // 查找std命名空间
  NamespaceDecl *StdNamespace = nullptr;
  auto StdIdentifier = &ast_context.Idents.get("std");
  auto Result = ast_context.getTranslationUnitDecl()->lookup(StdIdentifier);
  for (auto *Decl : Result)
    if (auto *NamespaceDecl = llvm::dyn_cast<clang::NamespaceDecl>(Decl)) {
      StdNamespace = NamespaceDecl;
      break;
    }

  if (!StdNamespace)
    return nullptr;

  // 尝试在std命名空间中查找coroutine_traits (C++20)
  auto TraitsIdentifier = &ast_context.Idents.get("coroutine_traits");
  auto TraitsResult = StdNamespace->lookup(TraitsIdentifier);
  for (auto *Decl : TraitsResult)
    if (auto *TmplDecl = llvm::dyn_cast<ClassTemplateDecl>(Decl))
      return TmplDecl;

  // 查找std::experimental命名空间
  NamespaceDecl *StdExpNamespace = nullptr;
  auto ExpIdentifier = &ast_context.Idents.get("experimental");
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
  auto ResumableIdentifier = &ast_context.Idents.get("resumable_traits");
  auto ResumableResult = StdExpNamespace->lookup(ResumableIdentifier);
  for (auto *Decl : ResumableResult)
    if (auto *TmplDecl = llvm::dyn_cast<ClassTemplateDecl>(Decl))
      return TmplDecl;

  return nullptr;
}

QualType getCoroutineTraitsType(const FunctionDecl *FD,
                                const ClassTemplateDecl *CTD,
                                ASTContext &ast_context) {
  if (!CTD)
    return QualType();

  llvm::SmallVector<TemplateArgument> TemplateArgs;

  // 1. 返回类型
  TemplateArgs.push_back(TemplateArgument(FD->getReturnType()));

  // 2. 参数类型
  for (const auto *Param : FD->parameters()) {
    TemplateArgs.push_back(TemplateArgument(Param->getType()));
  }

  // 3. this指针类型（成员函数）
  if (auto *Method = llvm::dyn_cast<CXXMethodDecl>(FD))
    if (!Method->isStatic())
      TemplateArgs.push_back(TemplateArgument(Method->getThisType()));

  // 创建特化类型
  return ast_context.getTemplateSpecializationType(
      TemplateName(const_cast<ClassTemplateDecl *>(CTD)),
      TemplateArgs,  // SpecifiedArgs
      TemplateArgs); // CanonicalArgs (coroutine_traits通常两者相同)
}

FunctionDecl *getCoroutineNewFunction(const FunctionDecl *FD) {
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

FunctionDecl *getCoroutineDeleteFunction(const FunctionDecl *FD) {
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
