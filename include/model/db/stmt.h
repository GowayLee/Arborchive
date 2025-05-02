#ifndef _MODEL_STMT_H_
#define _MODEL_STMT_H_

#include "db/cache_repository.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <string>

enum class StmtType {
  EXPR = 1,
  IF = 2,
  WHILE = 3,
  GOTO = 4,
  LABEL = 5,
  RETURN = 6,
  BLOCK = 7,
  END_TEST_WHILE = 8,
  FOR = 9,
  SWITCH_CASE = 10,
  SWITCH = 11,
  ASM = 13,
  TRY_BLOCK = 15,
  MICROSOFT_TRY = 16,
  DECL = 17,
  SET_VLA_SIZE = 18,
  VLA_DECL = 19,
  ASSIGNED_GOTO = 25,
  EMPTY = 26,
  CONTINUE = 27,
  BREAK = 28,
  RANGE_BASED_FOR = 29,
  HANDLER = 33,
  CONSTEXPR_IF = 35,
  CO_RETURN = 37,
  CONSTEVAL_IF = 38,
  NOT_CONSTEVAL_IF = 39
};

namespace DbModel {

struct Stmt {
  int id;
  int kind;
  int location;

  using KeyType = std::string;
  static KeyType makeKey(const clang::Stmt *stmt,
                         const clang::ASTContext &ctx) {
    std::string s;
    llvm::raw_string_ostream os(s);

    // Get Stmt type
    unsigned stmtClass = stmt->getStmtClass();
    os << stmtClass << "-";

    // Get src location
    const clang::SourceManager &srcMgr = ctx.getSourceManager();
    clang::SourceLocation beginLoc = stmt->getBeginLoc();
    clang::SourceLocation endLoc = stmt->getEndLoc();

    if (beginLoc.isValid() && endLoc.isValid()) {
      // Get expanded src location
      clang::SourceLocation expBegin = srcMgr.getExpansionLoc(beginLoc);
      clang::SourceLocation expEnd = srcMgr.getExpansionLoc(endLoc);

      // Get file ID
      std::pair<clang::FileID, unsigned> beginInfo =
          srcMgr.getDecomposedLoc(expBegin);
      std::pair<clang::FileID, unsigned> endInfo =
          srcMgr.getDecomposedLoc(expEnd);

      os << beginInfo.first.getHashValue() << "-";
      os << beginInfo.second << "-";
      os << endInfo.first.getHashValue() << "-";
      os << endInfo.second;
    } else {
      // Use ptr address as fallback
      os << "addr-" << reinterpret_cast<uintptr_t>(stmt);
    }

    // Special handling for certain statement types to enhance uniqueness
    if (auto declRefExpr = llvm::dyn_cast<clang::DeclRefExpr>(stmt)) {
      if (declRefExpr->getDecl()) {
        os << "-decl-" << declRefExpr->getDecl()->getID();
      }
    } else if (auto callExpr = llvm::dyn_cast<clang::CallExpr>(stmt)) {
      os << "-args-" << callExpr->getNumArgs();
    }

    return os.str();
  }
};

} // namespace DbModel

#define SEARCH_STMT_CACHE(type)                                                \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Stmt>>()                         \
      .find(type)

#endif // _MODEL_STMT_H_
