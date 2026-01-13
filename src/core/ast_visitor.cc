#include "core/ast_visitor.h"
#include <clang/AST/Decl.h>
#include <memory>

ASTVisitor::ASTVisitor(clang::ASTContext *context)
    : context_(context), pp_(context->getPrintingPolicy()) {
  initProcessors();
  pp_.SuppressScope = false;
  pp_.SuppressTagKeyword = true;
}

void ASTVisitor::initProcessors() {
  function_processor_ = std::make_unique<FunctionProcessor>(context_, pp_);
  namespace_processor_ = std::make_unique<NamespaceProcessor>(context_, pp_);
  variable_processor_ = std::make_unique<VariableProcessor>(context_, pp_);
  type_processor_ = std::make_unique<TypeProcessor>(context_, pp_);
  stmt_processor_ = std::make_unique<StmtProcessor>(context_, pp_);
  expr_processor_ = std::make_unique<ExprProcessor>(context_, pp_, type_processor_.get());
  specifier_processor_ = std::make_unique<SpecifierProcessor>(context_, pp_);
}

// 实现各种Visit方法

// Function Family
bool ASTVisitor::VisitFunctionDecl(clang::FunctionDecl *decl) {
  int func_id = function_processor_->routerProcess(decl);

  // Process function specifiers
  if (func_id != -1)
    specifier_processor_->processFunctionSpecifiers(func_id, decl);

  // Process return type with qualifiers
  int return_type_id =
      type_processor_->processType(decl->getReturnType().getTypePtr());
  specifier_processor_->processTypeQualifiers(return_type_id,
                                              decl->getReturnType());

  for (const auto &param : decl->parameters()) {
    int param_type_id =
        type_processor_->processType(param->getType().getTypePtr());
    specifier_processor_->processTypeQualifiers(param_type_id,
                                                param->getType());
  }
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
  int var_decl_id = variable_processor_->processVarDecl(decl);

  // Process variable specifiers
  if (var_decl_id != -1)
    specifier_processor_->processVariableSpecifiers(var_decl_id, decl);

  // Process type with qualifiers
  int type_id = type_processor_->processType(decl->getType().getTypePtr());
  specifier_processor_->processTypeQualifiers(type_id, decl->getType());
  return true;
}

bool ASTVisitor::VisitParmVarDecl(clang::ParmVarDecl *decl) {
  int var_decl_id = variable_processor_->processParmVarDecl(decl);

  // Process variable specifiers
  if (var_decl_id != -1)
    specifier_processor_->processVariableSpecifiers(var_decl_id, decl);

  // Process type with qualifiers
  int type_id = type_processor_->processType(decl->getType().getTypePtr());
  specifier_processor_->processTypeQualifiers(type_id, decl->getType());
  return true;
}

bool ASTVisitor::VisitFieldDecl(clang::FieldDecl *decl) {
  // Process type first to set _typeId in VariableProcessor
  int type_id = type_processor_->processType(decl->getType().getTypePtr());

  int var_decl_id = variable_processor_->processFieldDecl(decl);

  // Process variable specifiers
  if (var_decl_id != -1)
    specifier_processor_->processVariableSpecifiers(var_decl_id, decl);

  specifier_processor_->processTypeQualifiers(type_id, decl->getType());
  return true;
}

// Type Family
bool ASTVisitor::VisitRecordDecl(clang::RecordDecl *decl) {
  type_processor_->processRecordDecl(decl);
  return true;
}

bool ASTVisitor::VisitRecordType(clang::RecordType *RT) {
  type_processor_->processRecordType(RT);
  return true;
}

bool ASTVisitor::VisitEnumDecl(clang::EnumDecl *decl) {
  type_processor_->processEnumDecl(decl);
  return true;
}

bool ASTVisitor::VisitTypedefDecl(clang::TypedefDecl *decl) {
  type_processor_->processTypedefDecl(decl);
  return true;
}

bool ASTVisitor::VisitTemplateTypeParmDecl(clang::TemplateTypeParmDecl *decl) {
  type_processor_->processTemplateTypeParmDecl(decl);
  return true;
}

bool ASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *decl) {
  // LOG_DEBUG << "Visiting CXXRecordDecl" << std::endl;

  return true;
}

bool ASTVisitor::VisitBuiltinType(clang::BuiltinType *BT) {
  type_processor_->processBuiltinType(BT, context_);
  return true;
}

// TODO: Implement processing ImplicitCastExpr, record DerivedTypes of it
bool ASTVisitor::VisitImplicitCastExpr(clang::ImplicitCastExpr *ICE) {
  const clang::Type *sourceType = ICE->getSubExpr()->getType().getTypePtr();
  const clang::Type *targetType = ICE->getType().getTypePtr();

  int source_type_id = type_processor_->processType(sourceType);
  specifier_processor_->processTypeQualifiers(source_type_id,
                                              ICE->getSubExpr()->getType());

  int target_type_id = type_processor_->processType(targetType);
  specifier_processor_->processTypeQualifiers(target_type_id, ICE->getType());
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

bool ASTVisitor::VisitReturnStmt(clang::ReturnStmt *returnStmt) {
  stmt_processor_->processReturnStmt(returnStmt);
  return true;
}

bool ASTVisitor::VisitDeclStmt(clang::DeclStmt *declStmt) {
  stmt_processor_->processDeclStmt(declStmt);
  return true;
}

bool ASTVisitor::VisitCompoundStmt(clang::CompoundStmt *compoundStmt) {
  stmt_processor_->processBlockStmt(compoundStmt);
  return true;
}

bool ASTVisitor::VisitFriendDecl(clang::FriendDecl *decl) { return true; }

bool ASTVisitor::VisitTemplateDecl(clang::TemplateDecl *decl) { return true; }

bool ASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr *expr) {
  expr_processor_->processDeclRef(expr);
  return true;
}

bool ASTVisitor::VisitCallExpr(CallExpr *expr) {
  expr_processor_->processCallExpr(expr);
  return true;
}

bool ASTVisitor::VisitUnaryOperator(const UnaryOperator *op) {
  expr_processor_->processUnaryOperator(op);
  return true;
}

bool ASTVisitor::VisitBinaryOperator(const BinaryOperator *op) {
  expr_processor_->processBinaryOperator(op);
  return true;
}

bool ASTVisitor::VisitConditionalOperator(const ConditionalOperator *op) {
  expr_processor_->processConditionalOperator(op);
  return true;
}

bool ASTVisitor::VisitStringLiteral(const StringLiteral *literal) {
  expr_processor_->processStringLiteral(literal);
  return true;
}

bool ASTVisitor::VisitIntegerLiteral(const IntegerLiteral *literal) {
  expr_processor_->processIntegerLiteral(literal);
  return true;
}

bool ASTVisitor::VisitFloatingLiteral(const FloatingLiteral *literal) {
  expr_processor_->processFloatingLiteral(literal);
  return true;
}

bool ASTVisitor::VisitCharacterLiteral(const CharacterLiteral *literal) {
  expr_processor_->processCharacterLiteral(literal);
  return true;
}

bool ASTVisitor::VisitCXXBoolLiteralExpr(const CXXBoolLiteralExpr *literal) {
  expr_processor_->processBoolLiteral(literal);
  return true;
}

bool ASTVisitor::VisitNamespaceDecl(clang::NamespaceDecl *decl) {
  namespace_processor_->processNamespaceDecl(decl);
  return true;
}

bool ASTVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr *expr) {
  expr_processor_->processArraySubscriptExpr(expr);
  return true;
}

bool ASTVisitor::VisitInitListExpr(clang::InitListExpr *expr) {
  expr_processor_->processInitListExpr(expr);
  return true;
}

bool ASTVisitor::VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *expr) {
  expr_processor_->processUnaryExprOrTypeTraitExpr(expr);
  return true;
}
