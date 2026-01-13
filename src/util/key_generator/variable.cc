#include "util/key_generator/variable.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>
#include <sstream>
#include <string>

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Var {
// For clang::VarDecl
KeyType makeKey(const VarDecl *VD, ASTContext *ctx) {
  // 第一部分：处理源位置信息
  const SourceManager &srcMgr = ctx->getSourceManager();
  SourceLocation loc = srcMgr.getExpansionLoc(VD->getLocation());
  std::pair<FileID, unsigned> fileLoc = srcMgr.getDecomposedLoc(loc);
  unsigned line = srcMgr.getLineNumber(fileLoc.first, fileLoc.second);
  unsigned col = srcMgr.getColumnNumber(fileLoc.first, fileLoc.second);

  // 第二部分：构建基础ID（行:列:名称）
  std::string uid = std::to_string(line) + ":" + std::to_string(col) + ":" +
                    VD->getNameAsString();

  // 第三部分：添加声明上下文层次信息
  const DeclContext *hctx = VD->getDeclContext();
  std::string hierarchy;

  while (hctx && !hctx->isTranslationUnit()) {
    if (const auto *ns = dyn_cast<NamespaceDecl>(hctx)) {
      hierarchy =
          ns->getNameAsString() + (hierarchy.empty() ? "" : "::" + hierarchy);
    } else if (const auto *func = dyn_cast<FunctionDecl>(hctx)) {
      hierarchy =
          func->getNameAsString() + (hierarchy.empty() ? "" : "::" + hierarchy);
    }
    hctx = hctx->getParent();
  }

  if (!hierarchy.empty()) {
    uid += ":" + hierarchy;
  }

  return "var-" + uid;
}

// For clang::FieldDecl (MemberVar)
KeyType makeKey(const FieldDecl *FD, ASTContext *ctx) {
  // 第一部分：处理源位置信息
  const SourceManager &srcMgr = ctx->getSourceManager();
  SourceLocation loc = srcMgr.getExpansionLoc(FD->getLocation());
  std::pair<FileID, unsigned> fileLoc = srcMgr.getDecomposedLoc(loc);
  unsigned line = srcMgr.getLineNumber(fileLoc.first, fileLoc.second);
  unsigned col = srcMgr.getColumnNumber(fileLoc.first, fileLoc.second);

  // 第二部分：获取记录类型名称
  const RecordDecl *record = FD->getParent();
  std::string recordName = record ? record->getNameAsString() : "";

  // 处理嵌套类的层次结构
  const DeclContext *hctx = record ? record->getDeclContext() : nullptr;
  std::string hierarchy;

  while (hctx && !hctx->isTranslationUnit()) {
    if (const auto *ns = dyn_cast<NamespaceDecl>(hctx)) {
      std::string nsName = ns->getNameAsString();
      if (!nsName.empty()) {
        hierarchy = nsName + (hierarchy.empty() ? "" : "::" + hierarchy);
      }
    } else if (const auto *parentRecord = dyn_cast<RecordDecl>(hctx)) {
      std::string parentName = parentRecord->getNameAsString();
      if (!parentName.empty()) {
        hierarchy = parentName + (hierarchy.empty() ? "" : "::" + hierarchy);
      }
    }
    hctx = hctx->getParent();
  }

  // 构建完整的键: "member:line:col:recordname:fieldname[:hierarchy]"
  std::ostringstream oss;
  oss << "member:" << line << ":" << col << ":"
      << recordName << ":" << FD->getNameAsString();

  if (!hierarchy.empty()) {
    oss << ":" << hierarchy;
  }

  return oss.str();
}

} // namespace Var

} // namespace KeyGen
