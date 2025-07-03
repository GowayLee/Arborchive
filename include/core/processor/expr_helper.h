#ifndef _EXPR_HELPER_H_
#define _EXPR_HELPER_H_

#include "db/storage_facade.h"
#include "model/db/type.h"
#include "util/logger/macros.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>

using namespace clang;

// 检查是否在构造函数初始化列表中
bool isInCtorInitializer(DeclRefExpr *expr, ASTContext &context) {
  auto parents = context.getParents(*expr);
  while (!parents.empty()) {
    if (parents[0].get<CXXCtorInitializer>()) {
      return true;
    }
    parents = context.getParents(parents[0]);
  }
  return false;
}

// 检查是否在析构函数中
bool isInDtorContext(DeclRefExpr *expr, ASTContext &context) {
  if (auto *func = dyn_cast_or_null<FunctionDecl>(
          context.getParents(*expr)[0].get<Decl>())) {
    return func->isDestructor();
  }
  return false;
}

#endif // _EXPR_HELPER_H_
