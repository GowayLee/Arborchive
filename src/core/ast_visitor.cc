// ARCHITECTURE RULE:
//
// ASTVisitor is dispatch-only.
//
// Semantic orchestration MUST live inside processors/helpers.
// Do NOT expand subsystem coordination inside Visit*.
//
// Historical note:
// Template orchestration previously polluted this file and
// caused architecture degradation.
#include "core/ast_visitor.h"
#include "core/srcloc_recorder.h"
#include "db/dependency_manager.h"
#include "db/storage_facade.h"
#include "model/db/concept.h"
#include "model/db/declaration.h"
#include "model/db/function.h"
#include "model/db/type.h"
#include "model/db/variable.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/function.h"
#include "util/key_generator/type.h"
#include "util/key_generator/variable.h"
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/ASTConcept.h>
#include <clang/AST/ExprConcepts.h>
#include <clang/Basic/SourceManager.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeLoc.h>
#include <llvm/Support/raw_ostream.h>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

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
  template_processor_ = std::make_unique<TemplateProcessor>(
      context_, pp_, type_processor_.get(), expr_processor_.get(),
      variable_processor_.get());
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

  template_processor_->processFunctionTemplateSpecialization(decl, func_id);

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

  if (clang::TypeSourceInfo *typeInfo = decl->getTypeSourceInfo()) {
    auto templateTypeLoc = typeInfo->getTypeLoc()
                               .getAsAdjusted<clang::TemplateSpecializationTypeLoc>();
    template_processor_->recordClassTemplateArgumentValues(type_id,
                                                           templateTypeLoc);
  }

  template_processor_->processVarTemplateSpecialization(
      llvm::dyn_cast_or_null<clang::VarTemplateSpecializationDecl>(decl),
      var_decl_id);
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
  template_processor_->recordTemplateTypeConstraint(decl);
  return true;
}

bool ASTVisitor::VisitTemplateTemplateParmDecl(
    clang::TemplateTemplateParmDecl *decl) {
  type_processor_->processTemplateTemplateParmDecl(decl);
  return true;
}

bool ASTVisitor::VisitNonTypeTemplateParmDecl(
    clang::NonTypeTemplateParmDecl *decl) {
  if (!decl)
    return true;

  int exprId = expr_processor_->processNonTypeTemplateParmDecl(decl);
  if (exprId == -1)
    return true;

  if (template_processor_->shouldInsertNontypeTemplateParameter(exprId)) {
    DbModel::NonTypeTemplateParameter row = {exprId};
    STG.insertClassObj(row);
  }

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

bool ASTVisitor::VisitImplicitCastExpr(clang::ImplicitCastExpr *ICE) {
  if (!ICE || !ICE->getSubExpr())
    return true;

  expr_processor_->processImplicitCastExpr(ICE);

  const clang::QualType sourceType = ICE->getSubExpr()->getType();
  const clang::QualType targetType = ICE->getType();

  int source_type_id = type_processor_->processType(sourceType.getTypePtr());
  specifier_processor_->processTypeQualifiers(source_type_id, sourceType);

  int target_type_id = type_processor_->processType(targetType.getTypePtr());
  specifier_processor_->processTypeQualifiers(target_type_id, targetType);
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

bool ASTVisitor::VisitFriendDecl(clang::FriendDecl *decl) {
  template_processor_->processFriendDecl(decl);
  return true;
}

bool ASTVisitor::VisitConceptDecl(clang::ConceptDecl *decl) {
  template_processor_->resolveConceptTemplateId(decl, context_);
  return true;
}

// Template-specific work is handled in dedicated visitors:
// VisitClassTemplateDecl, VisitFunctionTemplateDecl, VisitVarTemplateDecl.
bool ASTVisitor::VisitTemplateDecl(clang::TemplateDecl *) { return true; }

bool ASTVisitor::VisitClassTemplateDecl(clang::ClassTemplateDecl *decl) {
  if (!decl)
    return true;

  const clang::CXXRecordDecl *templatedDecl = decl->getTemplatedDecl();
  if (!templatedDecl)
    return true;

  KeyType typeKey = KeyGen::Type::makeKey(templatedDecl, context_);
  if (!SEARCH_TYPE_CACHE(typeKey))
    type_processor_->processRecordDeclType(templatedDecl);

  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey)) {
    DbModel::IsClassTemplate isClassTemplate = {*cachedId};
    STG.insertClassObj(isClassTemplate);
  } else {
    PendingUpdate update{
        typeKey, CacheType::TYPE, [](int resolvedId) {
          DbModel::IsClassTemplate isClassTemplate = {resolvedId};
          STG.insertClassObj(isClassTemplate);
        }};
    DependencyManager::instance().addDependency(update);
  }

  return true;
}

bool ASTVisitor::VisitClassTemplateSpecializationDecl(
    clang::ClassTemplateSpecializationDecl *decl) {
  template_processor_->processClassTemplateSpecialization(decl);
  return true;
}

bool ASTVisitor::VisitFunctionTemplateDecl(clang::FunctionTemplateDecl *decl) {
  if (!decl)
    return true;

  const clang::FunctionDecl *templatedDecl = decl->getTemplatedDecl();
  if (!templatedDecl)
    return true;

  KeyType functionKey = KeyGen::Function::makeKey(templatedDecl, context_);
  if (auto cachedId = SEARCH_FUNCTION_CACHE(functionKey)) {
    DbModel::IsFunctionTemplate isFunctionTemplate = {*cachedId};
    STG.insertClassObj(isFunctionTemplate);
  } else {
    PendingUpdate update{
        functionKey, CacheType::FUNCTION, [](int resolvedId) {
          DbModel::IsFunctionTemplate isFunctionTemplate = {resolvedId};
          STG.insertClassObj(isFunctionTemplate);
        }};
    DependencyManager::instance().addDependency(update);
  }

  return true;
}

bool ASTVisitor::VisitVarTemplateDecl(clang::VarTemplateDecl *decl) {
  if (!decl)
    return true;

  clang::VarDecl *templatedDecl = decl->getTemplatedDecl();
  if (!templatedDecl)
    return true;

  int variableId = template_processor_->resolveVariableEntityId(templatedDecl);
  if (variableId == -1 ||
      !template_processor_->shouldInsertVariableTemplate(variableId))
    return true;

  DbModel::IsVariableTemplate isVariableTemplate = {variableId};
  STG.insertClassObj(isVariableTemplate);
  return true;
}

bool ASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr *expr) {
  if (!expr)
    return true;

  expr_processor_->processDeclRef(expr);

  if (expr->hasExplicitTemplateArgs()) {
    if (const auto *functionDecl =
            llvm::dyn_cast<clang::FunctionDecl>(expr->getDecl())) {
      KeyType functionKey = KeyGen::Function::makeKey(functionDecl, context_);
      int functionId = SEARCH_FUNCTION_CACHE(functionKey).value_or(-1);
      template_processor_->recordFunctionTemplateArgumentValues(
          functionId, expr->getTemplateArgs(), expr->getNumTemplateArgs());
    }
  }

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

bool ASTVisitor::VisitUsingDecl(clang::UsingDecl *decl) {
  namespace_processor_->processUsingDecl(decl);
  return true;
}

bool ASTVisitor::VisitUsingDirectiveDecl(clang::UsingDirectiveDecl *decl) {
  namespace_processor_->processUsingDirectiveDecl(decl);
  return true;
}

bool ASTVisitor::VisitUnresolvedUsingTypenameDecl(
    clang::UnresolvedUsingTypenameDecl *decl) {
  namespace_processor_->processUnresolvedUsingTypenameDecl(decl);
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

bool ASTVisitor::VisitConceptSpecializationExpr(
    clang::ConceptSpecializationExpr *expr) {
  template_processor_->processConceptSpecialization(expr);
  return true;
}
