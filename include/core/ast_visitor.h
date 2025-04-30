#ifndef _AST_VISITOR_H_
#define _AST_VISITOR_H_

#include "core/processor/class_processor.h"
#include "core/processor/function_processor.h"
#include "core/processor/location_processor.h"
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <memory>

class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
private:
  clang::ASTContext &context_;

  ////// Processors /////////
  std::unique_ptr<LocationProcessor> location_processor_ = nullptr;
  std::unique_ptr<ClassDeclProcessor> class_decl_processor_ = nullptr;
  std::unique_ptr<FunctionProcessor> function_processor_ = nullptr;

public:
  explicit ASTVisitor(clang::ASTContext &context);

  // 为各种AST节点类型实现Visit方法

  // 声明类型
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl);

  // Function Family
  bool VisitFunctionDecl(clang::FunctionDecl *decl);
  // bool VisitCXXMethodDecl(clang::CXXMethodDecl *decl);
  bool VisitCXXConstructorDecl(clang::CXXConstructorDecl *decl);
  bool VisitCXXDestructorDecl(clang::CXXDestructorDecl *decl);
  bool VisitCXXConversionDecl(clang::CXXConversionDecl *decl);
  bool VisitCXXDeductionGuideDecl(clang::CXXDeductionGuideDecl *decl);

  bool VisitTypeDecl(clang::TypeDecl *decl);
  bool VisitVarDecl(clang::VarDecl *decl);
  bool VisitEnumDecl(clang::EnumDecl *decl);
  bool VisitFriendDecl(clang::FriendDecl *decl);
  bool VisitTemplateDecl(clang::TemplateDecl *decl);

  // 语句类型
  bool VisitDeclStmt(clang::DeclStmt *stmt);
  bool VisitCompoundStmt(clang::CompoundStmt *stmt);

  // 表达式类型
  bool VisitDeclRefExpr(clang::DeclRefExpr *expr);
  bool VisitCallExpr(clang::CallExpr *expr);

  // 初始化处理器
  void initProcessors();
};

#endif // _AST_VISITOR_H_
