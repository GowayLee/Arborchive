#ifndef _KEY_GENERATOR_EXPR_H_
#define _KEY_GENERATOR_EXPR_H_

#include "db/cache_repository.h"
#include "model/db/expr.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Specifiers.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <string>

#define SEARCH_EXPR_CACHE(type)                                                \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Expr>>()                         \
      .find(type)

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Expr_ {
// For clang::Expr
static KeyType makeKey(const Expr *expr, const ASTContext &ctx) {
  std::string s;
  llvm::raw_string_ostream os(s);

  // Get Expr type (which is a Stmt subclass)
  unsigned exprClass = expr->getStmtClass();
  os << exprClass << "-";

  // Add expression-specific information: type and value category
  QualType qualType = expr->getType();
  if (!qualType.isNull()) {
    os << "type-" << qualType.getAsString() << "-";
  }

  // Add value category (lvalue, rvalue, etc.)
  ExprValueKind valueKind = expr->getValueKind();
  os << "vk-" << valueKind << "-";

  // Add object kind (ordinary or bitfield)
  ExprObjectKind objectKind = expr->getObjectKind();
  os << "ok-" << objectKind << "-";

  // Get src location (similar to Stmt implementation)
  const SourceManager &srcMgr = ctx.getSourceManager();
  SourceLocation beginLoc = expr->getBeginLoc();
  SourceLocation endLoc = expr->getEndLoc();

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
    os << "addr-" << reinterpret_cast<uintptr_t>(expr);
  }

  // Special handling for different expression types to enhance uniqueness
  if (auto declRefExpr = llvm::dyn_cast<DeclRefExpr>(expr)) {
    if (declRefExpr->getDecl()) {
      os << "-decl-" << declRefExpr->getDecl()->getID();
    }
  } else if (auto callExpr = llvm::dyn_cast<CallExpr>(expr)) {
    os << "-args-" << callExpr->getNumArgs();
    // Add function name if available
    if (auto *callee = callExpr->getDirectCallee()) {
      os << "-func-" << callee->getNameAsString();
    }
  } else if (auto binaryOp = llvm::dyn_cast<BinaryOperator>(expr)) {
    os << "-opcode-" << binaryOp->getOpcode();
  } else if (auto unaryOp = llvm::dyn_cast<UnaryOperator>(expr)) {
    os << "-opcode-" << unaryOp->getOpcode();
  } else if (auto castExpr = llvm::dyn_cast<CastExpr>(expr)) {
    os << "-kind-" << castExpr->getCastKind();
  } else if (auto literalExpr = llvm::dyn_cast<IntegerLiteral>(expr)) {
    // 对整数字面量，添加其值信息
    llvm::SmallString<20> valueStr;
    literalExpr->getValue().toString(valueStr, 10, true);
    os << "-value-" << valueStr;
  } else if (auto floatLiteral = llvm::dyn_cast<FloatingLiteral>(expr)) {
    // 对浮点字面量，添加其值信息
    llvm::SmallString<20> valueStr;
    floatLiteral->getValue().toString(valueStr);
    os << "-value-" << valueStr;
  } else if (auto stringLiteral = llvm::dyn_cast<StringLiteral>(expr)) {
    // 对字符串字面量，添加其长度信息（避免添加完整字符串可能导致键过长）
    os << "-len-" << stringLiteral->getLength();
    // 可选：添加字符串的哈希值
    os << "-hash-"
       << std::hash<std::string>{}(stringLiteral->getString().str());
  } else if (auto arraySubscriptExpr =
                 llvm::dyn_cast<ArraySubscriptExpr>(expr)) {
    // 对数组下标表达式，添加特殊标识
    os << "-array-subscript";
  } else if (auto memberExpr = llvm::dyn_cast<MemberExpr>(expr)) {
    // 对成员访问表达式，添加成员名称
    if (auto *field = memberExpr->getMemberDecl()) {
      os << "-member-" << field->getNameAsString();
    }
    os << "-isarrow-" << memberExpr->isArrow();
  }

  return os.str();
}

} // namespace Expr_

} // namespace KeyGen

#endif // _KEY_GENERATOR_EXPR_H_
