#ifndef _AST_VISITOR_H_
#define _AST_VISITOR_H_

#include "core/processor/class_processor.h"
#include "core/processor/expr_processor.h"
#include "core/processor/function_processor.h"
#include "core/processor/namespace_processor.h"
#include "core/processor/stmt_processor.h"
#include "core/processor/type_processor.h"
#include "core/processor/variable_processor.h"
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <memory>

class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
private:
  clang::ASTContext &context_;

  ////// Processors /////////
  std::unique_ptr<ClassDeclProcessor> class_decl_processor_ = nullptr;
  std::unique_ptr<FunctionProcessor> function_processor_ = nullptr;
  std::unique_ptr<NamespaceProcessor> namespace_processor_ = nullptr;
  std::unique_ptr<VariableProcessor> variable_processor_ = nullptr;
  std::unique_ptr<TypeProcessor> type_processor_ = nullptr;
  std::unique_ptr<StmtProcessor> stmt_processor_ = nullptr;
  std::unique_ptr<ExprProcessor> expr_processor_ = nullptr;

public:
  explicit ASTVisitor(clang::ASTContext &context);

  // 为各种AST节点类型实现Visit方法

  // 声明类型
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl);
  bool VisitNamespaceDecl(clang::NamespaceDecl *decl);

  // Function Family
  bool VisitFunctionDecl(clang::FunctionDecl *decl);
  // bool VisitCXXMethodDecl(clang::CXXMethodDecl *decl);
  bool VisitCXXConstructorDecl(clang::CXXConstructorDecl *decl);
  bool VisitCXXDestructorDecl(clang::CXXDestructorDecl *decl);
  bool VisitCXXConversionDecl(clang::CXXConversionDecl *decl);
  bool VisitCXXDeductionGuideDecl(clang::CXXDeductionGuideDecl *decl);

  // Variable Family
  bool VisitVarDecl(clang::VarDecl *decl);

  bool VisitTypeDecl(clang::TypeDecl *decl);
  bool VisitEnumDecl(clang::EnumDecl *decl);
  bool VisitFriendDecl(clang::FriendDecl *decl);
  bool VisitTemplateDecl(clang::TemplateDecl *decl);

  // Stmt Family
  bool VisitIfStmt(clang::IfStmt *ifStmt);
  bool VisitForStmt(clang::ForStmt *forStmt);
  bool VisitCXXForRangeStmt(clang::CXXForRangeStmt *rangeForStmt);
  bool VisitWhileStmt(clang::WhileStmt *whileStmt);
  bool VisitDoStmt(clang::DoStmt *doStmt);
  bool VisitSwitchStmt(clang::SwitchStmt *switchStmt);
  bool VisitDeclStmt(clang::DeclStmt *stmt);
  bool VisitCompoundStmt(clang::CompoundStmt *stmt);

  // Expr Family
  bool VisitDeclRefExpr(clang::DeclRefExpr *expr);
  bool VisitCallExpr(clang::CallExpr *expr);
  bool VisitUnaryOperator(const clang::UnaryOperator *op);
  bool VisitBinaryOperator(const clang::BinaryOperator *op);
  bool VisitConditionalOperator(const clang::ConditionalOperator *op);

  // Literal Family
  bool VisitStringLiteral(const clang::StringLiteral *literal);
  bool VisitIntegerLiteral(const clang::IntegerLiteral *literal);
  bool VisitFloatingLiteral(const clang::FloatingLiteral *literal);
  bool VisitCharacterLiteral(const clang::CharacterLiteral *literal);
  bool VisitCXXBoolLiteralExpr(const clang::CXXBoolLiteralExpr *literal);

  // 初始化处理器
  void initProcessors();
};

#endif // _AST_VISITOR_H_
