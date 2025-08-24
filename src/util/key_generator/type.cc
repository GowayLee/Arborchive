#include "util/key_generator/type.h"
#include "util/logger/macros.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Type.h>
#include <iostream>
#include <llvm/ADT/StringExtras.h>
#include <string>

namespace KeyGen {

namespace Type {

// For clang::QualType
KeyType makeKey(const QualType &qualType, const ASTContext &ctx) {
  std::string s;
  llvm::raw_string_ostream os(s);
  qualType.getCanonicalType().getUnqualifiedType().print(
      os, ctx.getPrintingPolicy());
  return os.str();
}

// For clang::NamedDecl
KeyType makeKey(const NamedDecl *decl, const ASTContext &ctx) {
  // 1. 作用域路径
  std::string scope = getScopePath(decl);
  // 2. 声明名称（含匿名处理）
  std::string name = getDeclName(decl);
  // 3. 模板参数
  std::string templateArgs = getTemplateArgs(decl);
  // 组合完整标识符
  std::string uid;
  if (!scope.empty())
    uid = scope + "::";
  uid += name + templateArgs;
  return "type-" + uid;
}

// clang::TemplateDecl -> clang::NamedDecl
KeyType makeKey(const TemplateDecl *decl, const ASTContext &ctx) {
  // return makeKey(cast<NamedDecl>(decl), ctx);
  if (auto *namedDecl = dyn_cast<NamedDecl>(decl))
    return makeKey(namedDecl, ctx);
  else {
    // 处理非NamedDecl情况
    LOG_WARNING << "Warning: Unexpected decl type: " << decl->getDeclKindName()
                << std::endl;
    return "";
  }
}

// clang::TypeDecl -> clang::NamedDecl
KeyType makeKey(const TypeDecl *decl, const ASTContext &ctx) {
  // return makeKey(cast<NamedDecl>(decl), ctx);
  if (auto *namedDecl = dyn_cast<NamedDecl>(decl))
    return makeKey(namedDecl, ctx);
  else {
    // 处理非NamedDecl情况
    LOG_WARNING << "Warning: Unexpected decl type: " << decl->getDeclKindName()
                << std::endl;
    return "";
  }
}

} // namespace Type

} // namespace KeyGen

// namespace "ns" -> 类 "A" -> 类 "B" 的作用域为 "ns::A::B"
std::string getScopePath(const NamedDecl *decl) {
  std::vector<std::string> scopes;
  const DeclContext *ctx = decl->getDeclContext();

  while (ctx && llvm::isa<NamedDecl>(ctx)) {
    const auto *namedCtx = llvm::cast<NamedDecl>(ctx);
    if (!namedCtx->getDeclName().isEmpty()) {
      scopes.push_back(namedCtx->getNameAsString());
    }
    ctx = ctx->getParent();
  }

  std::reverse(scopes.begin(), scopes.end());
  return llvm::join(scopes.begin(), scopes.end(), "::");
}

std::string getDeclName(const NamedDecl *decl) {
  if (decl->getDeclName().isEmpty()) {
    // 匿名声明：使用源码位置作为唯一标识
    SourceLocation loc = decl->getLocation();
    return "(anonymous@" +
           loc.printToString(decl->getASTContext().getSourceManager()) + ")";
  }
  return decl->getNameAsString();
}

std::string getTemplateArgs(const NamedDecl *decl) {
  std::string args;

  if (const auto *templateDecl = llvm::dyn_cast<TemplateDecl>(decl)) {
    // 处理普通模板声明（如 template<typename T> class A）
    const auto *paramList = templateDecl->getTemplateParameters();
    args = getTemplateParamListAsString(paramList);
  } else if (const auto *specDecl =
                 llvm::dyn_cast<ClassTemplateSpecializationDecl>(decl)) {
    // 处理模板特化（如 A<int>）
    args = getTemplateArgumentListAsString(specDecl->getTemplateArgs());
  }

  return args.empty() ? "" : "<" + args + ">";
}

// 获取模板参数列表的字符串表示（如 "T, U"）
std::string getTemplateParamListAsString(const TemplateParameterList *params) {
  std::vector<std::string> paramStrs;
  for (const NamedDecl *param : *params) {
    paramStrs.push_back(param->getNameAsString());
  }
  return llvm::join(paramStrs, ", ");
}

// 获取模板实参的字符串表示（如 "int, 3"）
std::string getTemplateArgumentListAsString(const TemplateArgumentList &args) {
  std::vector<std::string> argStrs;
  for (unsigned i = 0; i < args.size(); ++i) {
    llvm::raw_string_ostream os(argStrs.emplace_back());
    args[i].print(PrintingPolicy(LangOptions()), os, true);
  }
  return llvm::join(argStrs, ", ");
}
