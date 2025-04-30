#include "core/ast_visitor.h"
#include "core/processor/location_processor.h"
#include "util/logger/macros.h"
#include <memory>

ASTVisitor::ASTVisitor(clang::ASTContext &context) : context_(context) {
  initProcessors();
}

void ASTVisitor::initProcessors() {
  location_processor_ = std::make_unique<LocationProcessor>(&context_);
  class_decl_processor_ = std::make_unique<ClassDeclProcessor>(&context_);
  function_processor_ = std::make_unique<FunctionProcessor>(&context_);
}

// 实现各种Visit方法

bool ASTVisitor::VisitDeclStmt(clang::DeclStmt *stmt) {
  LOG_DEBUG << "Visiting DeclStmt" << std::endl;

  location_processor_->processStmt(stmt->getBeginLoc(), stmt->getEndLoc());

  return true;
}

bool ASTVisitor::VisitFunctionDecl(clang::FunctionDecl *decl) {

  LOG_DEBUG << "Visiting Base Function: " << decl->getNameAsString()
            << std::endl;

  // Hereby, function will process function of @operator @builtin_function,
  // @user_defined_function, @normal_function
  function_processor_->routerProcess(decl);
  return true;
}

bool ASTVisitor::VisitCXXConstructorDecl(clang::CXXConstructorDecl *decl) {
  LOG_DEBUG << "Visiting Constructor Function: " << decl->getNameAsString()
            << std::endl;

  function_processor_->processCXXConstructor(decl);

  return true;
}
bool ASTVisitor::VisitCXXDestructorDecl(clang::CXXDestructorDecl *decl) {
  LOG_DEBUG << "Visiting Destructor Function: " << decl->getNameAsString()
            << std::endl;

  function_processor_->processCXXDestructor(decl);
  return true;
}
bool ASTVisitor::VisitCXXConversionDecl(clang::CXXConversionDecl *decl) {
  LOG_DEBUG << "Visiting Conversion Function: " << decl->getNameAsString()
            << std::endl;

  function_processor_->processCXXConversion(decl);
  return true;
}
bool ASTVisitor::VisitCXXDeductionGuideDecl(
    clang::CXXDeductionGuideDecl *decl) {
  LOG_DEBUG << "Visiting Deduction Guide Function: " << decl->getNameAsString()
            << std::endl;

  function_processor_->processCXXDeductionGuide(decl);
  return true;
}

bool ASTVisitor::VisitTypeDecl(clang::TypeDecl *decl) { return true; }

bool ASTVisitor::VisitVarDecl(clang::VarDecl *decl) { return true; }

bool ASTVisitor::VisitEnumDecl(clang::EnumDecl *decl) { return true; }

bool ASTVisitor::VisitFriendDecl(clang::FriendDecl *decl) { return true; }

bool ASTVisitor::VisitTemplateDecl(clang::TemplateDecl *decl) { return true; }

bool ASTVisitor::VisitCompoundStmt(clang::CompoundStmt *stmt) {
  LOG_DEBUG << "Visiting CompoundStmt" << std::endl;

  return true;
}

bool ASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr *expr) {
  LOG_DEBUG << "Visiting DeclRefExpr" << std::endl;

  return true;
}

bool ASTVisitor::VisitCallExpr(clang::CallExpr *expr) {
  LOG_DEBUG << "Visiting CallExpr" << std::endl;

  return true;
}

bool ASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
  LOG_DEBUG << "Visiting CXXRecordDecl" << std::endl;

  return true;
}
