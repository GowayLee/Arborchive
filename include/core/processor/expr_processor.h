#ifndef _EXPR_PROCESSOR_H_
#define _EXPR_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/expr.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class ExprProcessor : public BaseProcessor {
public:

  void processDeclRef(DeclRefExpr* expr);

  void processUnaryOperator(const UnaryOperator *op);
  void processBinaryOperator(const BinaryOperator *op);
  void processConditionalOperator(const ConditionalOperator *op);



  ExprProcessor(ASTContext *ast_context) : BaseProcessor(ast_context) {};
  ~ExprProcessor() = default;

private:
  int _typeId;
  int _varId;
  int _varDeclId;
  std::string _name;

  int processBaseExpr(Expr *expr, ExprType exprType);
  void recordVarBindExpr(VarDecl* VD, DeclRefExpr *expr);

};

#endif // _EXPR_PROCESSOR_H_
