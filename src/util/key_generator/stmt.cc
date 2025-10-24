#include "util/key_generator/stmt.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <string>

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Stmt_ {
// For clang::Stmt
KeyType makeKey(const Stmt *stmt, ASTContext *ctx) {
  std::string s;
  llvm::raw_string_ostream os(s);

  // Get Stmt type
  unsigned stmtClass = stmt->getStmtClass();
  os << stmtClass << "-";

  // Get src location
  const SourceManager &srcMgr = ctx->getSourceManager();
  SourceLocation beginLoc = stmt->getBeginLoc();
  SourceLocation endLoc = stmt->getEndLoc();

  if (beginLoc.isValid() && endLoc.isValid()) {
    // Get expanded src location
    SourceLocation expBegin = srcMgr.getExpansionLoc(beginLoc);
    SourceLocation expEnd = srcMgr.getExpansionLoc(endLoc);

    // Get file information for begin location
    std::pair<FileID, unsigned> beginInfo = srcMgr.getDecomposedLoc(expBegin);
    unsigned beginLine =
        srcMgr.getLineNumber(beginInfo.first, beginInfo.second);
    unsigned beginCol =
        srcMgr.getColumnNumber(beginInfo.first, beginInfo.second);

    // Get file information for end location
    std::pair<FileID, unsigned> endInfo = srcMgr.getDecomposedLoc(expEnd);
    unsigned endLine = srcMgr.getLineNumber(endInfo.first, endInfo.second);
    unsigned endCol = srcMgr.getColumnNumber(endInfo.first, endInfo.second);

    // Format: beginLine:beginCol-endLine:endCol
    os << beginLine << ":" << beginCol << "-" << endLine << ":" << endCol;
  } else {
    // Use ptr address as fallback
    os << "addr-" << reinterpret_cast<uintptr_t>(stmt);
  }

  // Special handling for certain statement types to enhance uniqueness
  if (auto declRefExpr = llvm::dyn_cast<DeclRefExpr>(stmt)) {
    if (declRefExpr->getDecl()) {
      os << "-decl-" << declRefExpr->getDecl()->getID();
    }
  } else if (auto callExpr = llvm::dyn_cast<CallExpr>(stmt)) {
    os << "-args-" << callExpr->getNumArgs();
  }

  return "stmt-" + os.str();
}

} // namespace Stmt_

} // namespace KeyGen
