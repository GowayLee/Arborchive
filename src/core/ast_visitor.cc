#include "core/ast_visitor.h"
#include "core/srcloc_recorder.h"
#include "util/logger/macros.h"
#include <memory>

ASTVisitor::ASTVisitor(clang::ASTContext &context) : context_(context) {
  initProcessors();
}

void ASTVisitor::initProcessors() {
  class_decl_processor_ = std::make_unique<ClassDeclProcessor>(&context_);
  function_processor_ = std::make_unique<FunctionProcessor>(&context_);
  variable_processor_ = std::make_unique<VariableProcessor>(&context_);
  type_processor_ = std::make_unique<TypeProcessor>(&context_);
  stmt_processor_ = std::make_unique<StmtProcessor>(&context_);
  expr_processor_ = std::make_unique<ExprProcessor>(&context_);
}

// 实现各种Visit方法

bool ASTVisitor::VisitDeclStmt(clang::DeclStmt *stmt) {
  SrcLocRecorder::processStmt(cast<Stmt>(stmt), &context_);
  return true;
}

// Function Family
bool ASTVisitor::VisitFunctionDecl(clang::FunctionDecl *decl) {
  function_processor_->routerProcess(decl);
  return true;
}
bool ASTVisitor::VisitCXXConstructorDecl(clang::CXXConstructorDecl *decl) {
  function_processor_->processCXXConstructor(decl);
  return true;
}
bool ASTVisitor::VisitCXXDestructorDecl(clang::CXXDestructorDecl *decl) {
  function_processor_->processCXXDestructor(decl);
  return true;
}
bool ASTVisitor::VisitCXXConversionDecl(clang::CXXConversionDecl *decl) {
  function_processor_->processCXXConversion(decl);
  return true;
}
bool ASTVisitor::VisitCXXDeductionGuideDecl(
    clang::CXXDeductionGuideDecl *decl) {
  function_processor_->processCXXDeductionGuide(decl);
  return true;
}

// Variable Family
bool ASTVisitor::VisitVarDecl(clang::VarDecl *decl) {
  variable_processor_->routerProcess(decl);
  return true;
}

// Type Family
bool ASTVisitor::VisitTypeDecl(clang::TypeDecl *decl) {
  type_processor_->routerProcess(decl);
  return true;
}

// Stmt Family
bool ASTVisitor::VisitIfStmt(clang::IfStmt *ifStmt) {
  stmt_processor_->processIfStmt(ifStmt);
  return true;
}

bool ASTVisitor::VisitForStmt(clang::ForStmt *forStmt) {
  stmt_processor_->processForStmt(forStmt);
  return true;
}

bool ASTVisitor::VisitCXXForRangeStmt(clang::CXXForRangeStmt *rangeForStmt) {
  stmt_processor_->processCXXForRangeStmt(rangeForStmt);
  return true;
}

bool ASTVisitor::VisitWhileStmt(clang::WhileStmt *whileStmt) {
  stmt_processor_->processWhileStmt(whileStmt);
  return true;
}

bool ASTVisitor::VisitDoStmt(clang::DoStmt *doStmt) {
  stmt_processor_->processDoStmt(doStmt);
  return true;
}

bool ASTVisitor::VisitSwitchStmt(clang::SwitchStmt *switchStmt) {
  stmt_processor_->processSwitchStmt(switchStmt);
  return true;
}

bool ASTVisitor::VisitEnumDecl(clang::EnumDecl *decl) { return true; }

bool ASTVisitor::VisitFriendDecl(clang::FriendDecl *decl) { return true; }

bool ASTVisitor::VisitTemplateDecl(clang::TemplateDecl *decl) { return true; }

bool ASTVisitor::VisitCompoundStmt(clang::CompoundStmt *stmt) {
  // LOG_DEBUG << "Visiting CompoundStmt" << std::endl;

  return true;
}

bool ASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr *expr) {
  // LOG_DEBUG << "Visiting DeclRefExpr" << std::endl;

  return true;
}

bool ASTVisitor::VisitCallExpr(clang::CallExpr *expr) {
  // LOG_DEBUG << "Visiting CallExpr" << std::endl;

  return true;
}

bool ASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
  // LOG_DEBUG << "Visiting CXXRecordDecl" << std::endl;

  return true;
}
