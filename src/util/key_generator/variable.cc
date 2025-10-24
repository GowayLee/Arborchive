#include "util/key_generator/variable.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/SourceManager.h>
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

} // namespace Var

} // namespace KeyGen
