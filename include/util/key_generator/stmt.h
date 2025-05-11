#ifndef _KEY_GENERATOR_STMT_H_
#define _KEY_GENERATOR_STMT_H_

#include "db/cache_repository.h"
#include "model/db/stmt.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <string>

#define SEARCH_STMT_CACHE(type)                                                \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Stmt>>()                         \
      .find(type)

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Stmt_ {
// For clang::Stmt
static KeyType makeKey(const Stmt *stmt, const ASTContext &ctx) {
  std::string s;
  llvm::raw_string_ostream os(s);

  // Get Stmt type
  unsigned stmtClass = stmt->getStmtClass();
  os << stmtClass << "-";

  // Get src location
  const SourceManager &srcMgr = ctx.getSourceManager();
  SourceLocation beginLoc = stmt->getBeginLoc();
  SourceLocation endLoc = stmt->getEndLoc();

  if (beginLoc.isValid() && endLoc.isValid()) {
    // Get expanded src location
    SourceLocation expBegin = srcMgr.getExpansionLoc(beginLoc);
    SourceLocation expEnd = srcMgr.getExpansionLoc(endLoc);

    // Get file ID
    std::pair<FileID, unsigned> beginInfo = srcMgr.getDecomposedLoc(expBegin);
    std::pair<FileID, unsigned> endInfo = srcMgr.getDecomposedLoc(expEnd);

    os << beginInfo.first.getHashValue() << "-";
    os << beginInfo.second << "-";
    os << endInfo.first.getHashValue() << "-";
    os << endInfo.second;
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

  return os.str();
}

} // namespace Stmt_

} // namespace KeyGen

#endif // _KEY_GENERATOR_STMT_H_
