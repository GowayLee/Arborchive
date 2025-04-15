#include "core/ast_visitor.h"
#include "core/processor/location_processor.h"
#include "util/logger/macros.h"

ASTVisitor::ASTVisitor(clang::ASTContext &context) : context_(context) {
  initProcessors();
}

void ASTVisitor::initProcessors() {
  // 创建各种处理器实例并设置AST上下文
  auto declStmtProcessor =
      BaseProcessor::createProcessor(getProcessorTypeID<DeclStmtProcessor>());
  if (declStmtProcessor) {
    declStmtProcessor->setASTContext(&context_);
    processors_.push_back(std::move(declStmtProcessor));
  }

  auto compoundStmtProcessor = BaseProcessor::createProcessor(
      getProcessorTypeID<CompoundStmtProcessor>());
  if (compoundStmtProcessor) {
    compoundStmtProcessor->setASTContext(&context_);
    processors_.push_back(std::move(compoundStmtProcessor));
  }

  auto declRefExprProcessor = BaseProcessor::createProcessor(
      getProcessorTypeID<DeclRefExprProcessor>());
  if (declRefExprProcessor) {
    declRefExprProcessor->setASTContext(&context_);
    processors_.push_back(std::move(declRefExprProcessor));
  }

  auto callExprProcessor =
      BaseProcessor::createProcessor(getProcessorTypeID<CallExprProcessor>());
  if (callExprProcessor) {
    callExprProcessor->setASTContext(&context_);
    processors_.push_back(std::move(callExprProcessor));
  }

  // 添加其他处理器...
}

// 实现各种Visit方法

bool ASTVisitor::VisitDeclStmt(clang::DeclStmt *stmt) {
  LOG_DEBUG << "Visiting DeclStmt" << std::endl;

  for (auto &processor : processors_) {
    processor->handleStmt(stmt);
  }

  return true; // 继续访问
}

bool ASTVisitor::VisitCompoundStmt(clang::CompoundStmt *stmt) {
  LOG_DEBUG << "Visiting CompoundStmt" << std::endl;

  for (auto &processor : processors_) {
    processor->handleStmt(stmt);
  }

  return true;
}

bool ASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr *expr) {
  LOG_DEBUG << "Visiting DeclRefExpr" << std::endl;

  for (auto &processor : processors_) {
    processor->handleExpr(expr);
  }

  return true;
}

bool ASTVisitor::VisitCallExpr(clang::CallExpr *expr) {
  LOG_DEBUG << "Visiting CallExpr" << std::endl;

  for (auto &processor : processors_) {
    processor->handleExpr(expr);
  }

  return true;
}

// 以下是其他访问方法的实现，我们也可以根据需要添加

bool ASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
  LOG_DEBUG << "Visiting CXXRecordDecl" << std::endl;

  for (auto &processor : processors_) {
    processor->handleDecl(decl);
  }

  return true;
}

bool ASTVisitor::VisitFunctionDecl(clang::FunctionDecl *decl) {
  LOG_DEBUG << "Visiting FunctionDecl" << std::endl;

  for (auto &processor : processors_) {
    processor->handleDecl(decl);
  }

  return true;
}

bool ASTVisitor::VisitVarDecl(clang::VarDecl *decl) {
  LOG_DEBUG << "Visiting VarDecl" << std::endl;

  for (auto &processor : processors_) {
    processor->handleDecl(decl);
  }

  return true;
}