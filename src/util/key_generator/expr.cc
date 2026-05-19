#include "util/key_generator/expr.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprConcepts.h>
#include <clang/AST/ExprCXX.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Specifiers.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

namespace KeyGen {

namespace Expr_ {
// For clang::Expr
// KeyType makeKey(const Expr *expr, const ASTContext &ctx) {
//   std::string s;
//   llvm::raw_string_ostream os(s);

//   // Get Expr type (which is a Stmt subclass)
//   unsigned exprClass = expr->getStmtClass();
//   os << exprClass << "-";

//   // Add expression-specific information: type and value category
//   QualType qualType = expr->getType();
//   if (!qualType.isNull()) {
//     os << "type-" << qualType.getAsString() << "-";
//   }

//   // Add value category (lvalue, rvalue, etc.)
//   ExprValueKind valueKind = expr->getValueKind();
//   os << "vk-" << valueKind << "-";

//   // Add object kind (ordinary or bitfield)
//   ExprObjectKind objectKind = expr->getObjectKind();
//   os << "ok-" << objectKind << "-";

//   // Get src location (similar to Stmt implementation)
//   const SourceManager &srcMgr = ctx.getSourceManager();
//   SourceLocation beginLoc = expr->getBeginLoc();
//   SourceLocation endLoc = expr->getEndLoc();

//   if (beginLoc.isValid() && endLoc.isValid()) {
//     // Get expanded src location
//     SourceLocation expBegin = srcMgr.getExpansionLoc(beginLoc);
//     SourceLocation expEnd = srcMgr.getExpansionLoc(endLoc);

//     // Get file ID
//     std::pair<FileID, unsigned> beginInfo =
//     srcMgr.getDecomposedLoc(expBegin); std::pair<FileID, unsigned> endInfo =
//     srcMgr.getDecomposedLoc(expEnd);

//     os << beginInfo.first.getHashValue() << "-";
//     os << beginInfo.second << "-";
//     os << endInfo.first.getHashValue() << "-";
//     os << endInfo.second;
//   } else {
//     // Use ptr address as fallback
//     os << "addr-" << reinterpret_cast<uintptr_t>(expr);
//   }

//   // Special handling for different expression types to enhance uniqueness
//   if (auto declRefExpr = llvm::dyn_cast<DeclRefExpr>(expr)) {
//     if (declRefExpr->getDecl()) {
//       os << "-decl-" << declRefExpr->getDecl()->getID();
//     }
//   } else if (auto callExpr = llvm::dyn_cast<CallExpr>(expr)) {
//     os << "-args-" << callExpr->getNumArgs();
//     // Add function name if available
//     if (auto *callee = callExpr->getDirectCallee()) {
//       os << "-func-" << callee->getNameAsString();
//     }
//   } else if (auto binaryOp = llvm::dyn_cast<BinaryOperator>(expr)) {
//     os << "-opcode-" << binaryOp->getOpcode();
//   } else if (auto unaryOp = llvm::dyn_cast<UnaryOperator>(expr)) {
//     os << "-opcode-" << unaryOp->getOpcode();
//   } else if (auto castExpr = llvm::dyn_cast<CastExpr>(expr)) {
//     os << "-kind-" << castExpr->getCastKind();
//   } else if (auto literalExpr = llvm::dyn_cast<IntegerLiteral>(expr)) {
//     // 对整数字面量，添加其值信息
//     llvm::SmallString<20> valueStr;
//     literalExpr->getValue().toString(valueStr, 10, true);
//     os << "-value-" << valueStr;
//   } else if (auto floatLiteral = llvm::dyn_cast<FloatingLiteral>(expr)) {
//     // 对浮点字面量，添加其值信息
//     llvm::SmallString<20> valueStr;
//     floatLiteral->getValue().toString(valueStr);
//     os << "-value-" << valueStr;
//   } else if (auto stringLiteral = llvm::dyn_cast<StringLiteral>(expr)) {
//     // 对字符串字面量，添加其长度信息（避免添加完整字符串可能导致键过长）
//     os << "-len-" << stringLiteral->getLength();
//     // 可选：添加字符串的哈希值
//     os << "-hash-"
//        << std::hash<std::string>{}(stringLiteral->getString().str());
//   } else if (auto arraySubscriptExpr =
//                  llvm::dyn_cast<ArraySubscriptExpr>(expr)) {
//     // 对数组下标表达式，添加特殊标识
//     os << "-array-subscript";
//   } else if (auto memberExpr = llvm::dyn_cast<MemberExpr>(expr)) {
//     // 对成员访问表达式，添加成员名称
//     if (auto *field = memberExpr->getMemberDecl()) {
//       os << "-member-" << field->getNameAsString();
//     }
//     os << "-isarrow-" << memberExpr->isArrow();
//   }

//   return os.str();
// }

KeyType makeKey(const Expr *expr, ASTContext *ctx) {
  const SourceManager &SM = ctx->getSourceManager();

  auto range = expr->getSourceRange();
  auto start = SM.getSpellingLoc(range.getBegin());
  auto end = SM.getSpellingLoc(range.getEnd());
  // 获取位置信息
  unsigned startLine = SM.getSpellingLineNumber(start);
  unsigned startCol = SM.getSpellingColumnNumber(start);
  unsigned endLine = SM.getSpellingLineNumber(end);
  unsigned endCol = SM.getSpellingColumnNumber(end);

  // 生成唯一字符串
  std::string locStr = llvm::Twine("expr-")
                           .concat(std::to_string(startLine))
                           .concat("-")
                           .concat(std::to_string(startCol))
                           .concat("-")
                           .concat(std::to_string(endLine))
                           .concat("-")
                           .concat(std::to_string(endCol))
                           .str();

  // Add expression-specific information to enhance uniqueness
  if (auto callExpr = llvm::dyn_cast<CallExpr>(expr)) {
    locStr += "-args-" + std::to_string(callExpr->getNumArgs());
    // Add function name if available
    if (auto *callee = callExpr->getDirectCallee()) {
      locStr += "-func-" + callee->getNameAsString();
    }
  } else if (auto declRefExpr = llvm::dyn_cast<DeclRefExpr>(expr)) {
    if (declRefExpr->getDecl()) {
      locStr += "-decl-" + std::to_string(declRefExpr->getDecl()->getID());
    }
  } else if (auto binaryOp = llvm::dyn_cast<BinaryOperator>(expr)) {
    locStr += "-opcode-" + std::to_string(binaryOp->getOpcode());
  } else if (auto unaryOp = llvm::dyn_cast<UnaryOperator>(expr)) {
    locStr += "-opcode-" + std::to_string(unaryOp->getOpcode());
  } else if (auto conceptExpr =
                 llvm::dyn_cast<ConceptSpecializationExpr>(expr)) {
    std::string fileName = SM.getFilename(start).str();
    if (!fileName.empty())
      locStr += "-file-" + fileName;

    if (const ConceptDecl *concept = conceptExpr->getNamedConcept()) {
      const ConceptDecl *canonicalConcept = concept->getCanonicalDecl();
      locStr += "-concept-" +
                canonicalConcept->getQualifiedNameAsString();
    }

    // Keep this aligned with the concept specialization identity key: use the
    // printed template argument content, not only the argument count.
    std::string argKey;
    llvm::raw_string_ostream argStream(argKey);
    clang::PrintingPolicy policy(ctx->getLangOpts());
    llvm::ArrayRef<clang::TemplateArgument> args =
        conceptExpr->getTemplateArguments();
    for (unsigned index = 0; index < args.size(); ++index) {
      if (index != 0)
        argStream << ",";
      args[index].print(policy, argStream, true);
    }
    locStr += "-args-" + argStream.str();
  }
  // // 生成MD5哈希
  // llvm::MD5 hash;
  // hash.update(locStr);
  // llvm::MD5::MD5Result result;
  // hash.final(result);

  // // 转换为十六进制字符串
  // llvm::SmallString<32> hexStr;
  // llvm::MD5::stringifyResult(result, hexStr);
  return locStr;
}

KeyType makeKeyForNonTypeTemplateParm(const NonTypeTemplateParmDecl *decl,
                                        ASTContext *ctx) {
  if (!decl || !ctx)
    return "";

  const SourceManager &SM = ctx->getSourceManager();

  // Source location
  SourceLocation loc = SM.getSpellingLoc(decl->getLocation());
  std::string locStr;
  if (loc.isValid()) {
    locStr = loc.printToString(SM);
  } else {
    locStr = "addr-" + std::to_string(reinterpret_cast<std::uintptr_t>(decl));
  }

  // Decl context (parent template or enclosing decl)
  std::string contextStr;
  if (const auto *dc = decl->getDeclContext()) {
    if (const auto *named = llvm::dyn_cast<NamedDecl>(dc)) {
      contextStr = named->getQualifiedNameAsString();
    } else {
      contextStr = std::to_string(reinterpret_cast<std::uintptr_t>(dc));
    }
  }

  // Depth and index
  unsigned depth = decl->getDepth();
  unsigned index = decl->getIndex();

  // Parameter name
  std::string name = decl->getNameAsString();

  // Parameter type
  std::string typeStr = decl->getType().getAsString();

  return "nttp-" + locStr + "-ctx-" + contextStr +
         "-depth-" + std::to_string(depth) +
         "-idx-" + std::to_string(index) +
         "-name-" + name +
         "-type-" + typeStr;
}

} // namespace Expr_

} // namespace KeyGen
