#ifndef _EXPR_PROCESSOR_H_
#define _EXPR_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/expr.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class TypeProcessor;

class ExprProcessor : public BaseProcessor {
public:
  void processDeclRef(DeclRefExpr *expr);

  void processUnaryOperator(const UnaryOperator *op);
  void processBinaryOperator(const BinaryOperator *op);
  void processConditionalOperator(const ConditionalOperator *op);

  void processStringLiteral(const StringLiteral *literal);
  void processIntegerLiteral(const IntegerLiteral *literal);
  void processFloatingLiteral(const FloatingLiteral *literal);
  void processCharacterLiteral(const CharacterLiteral *literal);
  void processBoolLiteral(const CXXBoolLiteralExpr *literal);

  void processAssignArithExpr(const BinaryOperator *op);
  void processAssignBitwiseExpr(const BinaryOperator *op);
  void processAssignPointerExpr(const BinaryOperator *op);
  void processAssignOpExpr(const BinaryOperator *op);
  void processAssignExpr(const BinaryOperator *op);
  void processCallExpr(const CallExpr *expr);

  void processImplicitCastExpr(const ImplicitCastExpr *ICE);

  void processArraySubscriptExpr(const ArraySubscriptExpr *expr);
  void processInitListExpr(const InitListExpr *expr);
  void processUnaryExprOrTypeTraitExpr(const UnaryExprOrTypeTraitExpr *expr);

  ExprProcessor(ASTContext *ast_context, const PrintingPolicy pp, TypeProcessor *tp = nullptr)
      : BaseProcessor(ast_context, pp), type_processor_(tp) {};
  ~ExprProcessor() = default;

private:
  int _typeId;
  int _varId;
  int _varDeclId;
  std::string _name;

  int processBaseExpr(Expr *expr, ExprKind exprKind);
  void recordVarBindExpr(VarDecl *VD, DeclRefExpr *expr);

  int processLiteralValue(const std::string &value, const std::string &text,
                          int exprId);
  void recordValueBindExpr(int valueId, int exprId);

  void recordAggregateArrayInit(int initListExprId, const InitListExpr *expr);
  void recordAggregateFieldInit(int initListExprId, const InitListExpr *expr);
  void recordSizeOfBind(int exprId, const UnaryExprOrTypeTraitExpr *expr);

  TypeProcessor *type_processor_ = nullptr;
};

#endif // _EXPR_PROCESSOR_H_
