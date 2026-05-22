#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include "core/processor/template_processor.h"
#include "core/processor/expr_processor.h"
#include "core/processor/type_processor.h"
#include "core/processor/variable_processor.h"
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
#include <clang/AST/ASTConcept.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/ExprConcepts.h>
#include <clang/AST/TypeLoc.h>
#include <clang/Basic/SourceManager.h>
#include <cstdint>
#include <llvm/Support/raw_ostream.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace {

struct ConceptSpecializationId {
  int id;
};

} // namespace

std::string TemplateProcessor::makePairKey(int first, int second) {
  return std::to_string(first) + ":" + std::to_string(second);
}

std::string TemplateProcessor::makeTripleKey(int first, int second,
                                             int third) {
  return std::to_string(first) + ":" + std::to_string(second) + ":" +
         std::to_string(third);
}

bool TemplateProcessor::shouldInsertClassInstantiation(int to, int from) {
  return classInstantiationDedup.insert(makePairKey(to, from)).second;
}

bool TemplateProcessor::shouldInsertClassTemplateArgument(int typeId, int index,
                                                          int argType) {
  return classTemplateArgumentDedup
      .insert(makeTripleKey(typeId, index, argType))
      .second;
}

bool TemplateProcessor::shouldInsertClassTemplateArgumentValue(int typeId,
                                                               int index,
                                                               int argValue) {
  return classTemplateArgumentValueDedup
      .insert(makeTripleKey(typeId, index, argValue))
      .second;
}

bool TemplateProcessor::shouldInsertFunctionInstantiation(int to, int from) {
  return functionInstantiationDedup.insert(makePairKey(to, from)).second;
}

bool TemplateProcessor::shouldInsertFunctionTemplateArgument(int functionId,
                                                             int index,
                                                             int argType) {
  return functionTemplateArgumentDedup
      .insert(makeTripleKey(functionId, index, argType))
      .second;
}

bool TemplateProcessor::shouldInsertFunctionTemplateArgumentValue(
    int functionId, int index, int argValue) {
  return functionTemplateArgumentValueDedup
      .insert(makeTripleKey(functionId, index, argValue))
      .second;
}

bool TemplateProcessor::shouldInsertVariableTemplate(int variableId) {
  return variableTemplateDedup.insert(std::to_string(variableId)).second;
}

bool TemplateProcessor::shouldInsertVariableInstantiation(int to, int from) {
  return variableInstantiationDedup
      .insert(makePairKey(to, from))
      .second;
}

bool TemplateProcessor::shouldInsertVariableTemplateArgument(int variableId,
                                                             int index,
                                                             int argType) {
  return variableTemplateArgumentDedup
      .insert(makeTripleKey(variableId, index, argType))
      .second;
}

bool TemplateProcessor::shouldInsertVariableTemplateArgumentValue(
    int variableId, int index, int argValue) {
  return variableTemplateArgumentValueDedup
      .insert(makeTripleKey(variableId, index, argValue))
      .second;
}

bool TemplateProcessor::shouldInsertTemplateTemplateInstantiation(int to,
                                                                  int from) {
  return templateTemplateInstantiationDedup.insert(makePairKey(to, from))
      .second;
}

bool TemplateProcessor::shouldInsertTemplateTemplateArgument(int typeId,
                                                             int index,
                                                             int argType) {
  return templateTemplateArgumentDedup
      .insert(makeTripleKey(typeId, index, argType))
      .second;
}

bool TemplateProcessor::shouldInsertConceptInstantiation(int conceptId,
                                                         int templateId) {
  return conceptInstantiationDedup.insert(makePairKey(conceptId, templateId))
      .second;
}

bool TemplateProcessor::shouldInsertConceptTemplateArgument(int conceptId,
                                                            int index,
                                                            int argType) {
  return conceptTemplateArgumentDedup
      .insert(makeTripleKey(conceptId, index, argType))
      .second;
}

bool TemplateProcessor::shouldInsertTypeTemplateTypeConstraint(
    int typeId, int constraintId) {
  return typeTemplateTypeConstraintDedup
      .insert(makePairKey(typeId, constraintId))
      .second;
}

bool TemplateProcessor::shouldInsertIsTypeConstraint(int conceptId) {
  return isTypeConstraintDedup.insert(conceptId).second;
}

bool TemplateProcessor::shouldInsertNontypeTemplateParameter(int id) {
  return nontypeTemplateParameterDedup.insert(id).second;
}

bool TemplateProcessor::shouldInsertConceptTemplateArgumentValue(
    int conceptId, int index, int argValue) {
  return conceptTemplateArgumentValueDedup
      .insert(makeTripleKey(conceptId, index, argValue))
      .second;
}

std::string
TemplateProcessor::makeConceptTemplateKey(const clang::ConceptDecl *decl,
                                          clang::ASTContext *context) {
  if (!decl || !context)
    return "";

  const clang::ConceptDecl *canonicalDecl = decl->getCanonicalDecl();
  return KeyGen::Type::makeKey(canonicalDecl, context);
}

int TemplateProcessor::resolveConceptTemplateId(
    const clang::ConceptDecl *decl, clang::ASTContext *context) {
  if (!decl || !context)
    return -1;

  const clang::ConceptDecl *canonicalDecl = decl->getCanonicalDecl();
  std::string conceptKey = makeConceptTemplateKey(canonicalDecl, context);
  if (conceptKey.empty())
    return -1;

  if (auto it = conceptTemplateIds.find(conceptKey);
      it != conceptTemplateIds.end())
    return it->second;

  LocIdPair *locIdPair = SrcLocRecorder::processDefault(canonicalDecl, context);
  DbModel::ConceptTemplate conceptTemplate = {
      GENID(ConceptTemplate), canonicalDecl->getNameAsString(),
      locIdPair->spec_id};

  conceptTemplateIds.emplace(conceptKey, conceptTemplate.id);
  STG.insertClassObj(conceptTemplate);
  return conceptTemplate.id;
}

std::string TemplateProcessor::makeTemplateArgumentListKey(
    llvm::ArrayRef<clang::TemplateArgument> args,
    clang::ASTContext *context) {
  std::string key;
  llvm::raw_string_ostream os(key);
  clang::PrintingPolicy policy(context->getLangOpts());

  for (unsigned index = 0; index < args.size(); ++index) {
    if (index != 0)
      os << ",";
    args[index].print(policy, os, true);
  }

  return os.str();
}

std::string TemplateProcessor::makeConceptSpecializationKey(
    const clang::ConceptSpecializationExpr *expr,
    clang::ASTContext *context) {
  if (!expr || !context)
    return "";

  std::string conceptKey = makeConceptTemplateKey(expr->getNamedConcept(),
                                                  context);
  if (conceptKey.empty())
    return "";

  const clang::SourceManager &sourceManager = context->getSourceManager();
  clang::SourceLocation beginLoc =
      sourceManager.getSpellingLoc(expr->getBeginLoc());
  clang::SourceLocation endLoc =
      sourceManager.getSpellingLoc(expr->getEndLoc());

  std::string locationKey;
  if (beginLoc.isValid() && endLoc.isValid()) {
    locationKey =
        beginLoc.printToString(sourceManager) + "-" +
        endLoc.printToString(sourceManager);
  } else if (const auto *specializationDecl = expr->getSpecializationDecl()) {
    locationKey = "implicit-decl-" + std::to_string(specializationDecl->getID());
  } else {
    locationKey =
        "addr-" + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
  }

  return conceptKey + "<" +
         makeTemplateArgumentListKey(expr->getTemplateArguments(), context) +
         ">@" + locationKey;
}

int TemplateProcessor::resolveConceptSpecializationId(
    const clang::ConceptSpecializationExpr *expr,
    clang::ASTContext *context) {
  std::string specializationKey = makeConceptSpecializationKey(expr, context);
  if (specializationKey.empty())
    return -1;

  if (auto it = conceptSpecializationIds.find(specializationKey);
      it != conceptSpecializationIds.end())
    return it->second;

  int conceptId = IDGenerator::generateId<ConceptSpecializationId>();
  conceptSpecializationIds.emplace(specializationKey, conceptId);
  return conceptId;
}

int TemplateProcessor::resolveVariableEntityId(const clang::VarDecl *decl) {
  if (!decl || !variable_processor_ || !ast_context_)
    return -1;

  KeyType variableKey = KeyGen::Var::makeKey(decl, ast_context_);
  if (auto cachedId = SEARCH_VARIABLE_CACHE(variableKey))
    return *cachedId;

  variable_processor_->processVarDecl(decl);
  if (auto cachedId = SEARCH_VARIABLE_CACHE(variableKey))
    return *cachedId;

  return -1;
}

int TemplateProcessor::resolveTemplateArgumentTypeId(clang::QualType argType) {
  if (argType.isNull() || !type_processor_ || !ast_context_)
    return -1;

  KeyType typeKey = KeyGen::Type::makeKey(argType, ast_context_);
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    return *cachedId;

  if (const auto *builtinType = argType->getAs<clang::BuiltinType>())
    return type_processor_->processBuiltinType(builtinType, ast_context_);

  if (const auto *recordType = argType->getAs<clang::RecordType>()) {
    type_processor_->processRecordType(recordType);
    if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
      return *cachedId;
  }

  int typeId = type_processor_->processType(argType.getTypePtr());
  if (typeId != -1)
    return typeId;

  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    return *cachedId;

  return -1;
}

int TemplateProcessor::resolveTemplateTemplateParmId(
    const clang::TemplateTemplateParmDecl *decl) {
  if (!decl || !type_processor_ || !ast_context_)
    return -1;

  KeyType typeKey = KeyGen::Type::makeKey(decl, ast_context_);
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    return *cachedId;

  int typeId = type_processor_->processTemplateTemplateParmDecl(decl);
  if (typeId != -1)
    return typeId;

  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    return *cachedId;

  return -1;
}

int TemplateProcessor::resolveTemplateTemplateArgumentTypeId(
    const clang::TemplateArgument &arg) {
  if (arg.getKind() != clang::TemplateArgument::Template || !type_processor_ ||
      !ast_context_)
    return -1;

  const clang::TemplateDecl *templateDecl =
      arg.getAsTemplate().getAsTemplateDecl();
  const auto *classTemplateDecl =
      llvm::dyn_cast_or_null<clang::ClassTemplateDecl>(templateDecl);
  if (!classTemplateDecl || !classTemplateDecl->getTemplatedDecl())
    return -1;

  const clang::CXXRecordDecl *templatedDecl =
      classTemplateDecl->getTemplatedDecl();
  KeyType templateTypeKey = KeyGen::Type::makeKey(templatedDecl, ast_context_);
  if (auto cachedId = SEARCH_TYPE_CACHE(templateTypeKey))
    return *cachedId;

  int templateTypeId = type_processor_->processRecordDeclType(templatedDecl);
  if (templateTypeId != -1)
    return templateTypeId;

  if (auto cachedId = SEARCH_TYPE_CACHE(templateTypeKey))
    return *cachedId;

  return -1;
}

const clang::Expr *TemplateProcessor::getTemplateArgumentSourceExpr(
    const clang::TemplateArgumentLoc &argLoc) {
  const clang::TemplateArgument &arg = argLoc.getArgument();

  switch (arg.getKind()) {
  case clang::TemplateArgument::Expression:
    return argLoc.getSourceExpression();
  case clang::TemplateArgument::Integral:
    return argLoc.getSourceIntegralExpression();
  default:
    return nullptr;
  }
}

int TemplateProcessor::resolveTemplateArgumentExprId(
    const clang::Expr *sourceExpr) {
  if (!sourceExpr || !expr_processor_ || !ast_context_)
    return -1;

  const clang::Expr *expr = sourceExpr->IgnoreParenImpCasts();
  KeyType exprKey = KeyGen::Expr_::makeKey(expr, ast_context_);
  if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
    return *cachedId;

  if (const auto *integerLiteral =
          llvm::dyn_cast<clang::IntegerLiteral>(expr)) {
    expr_processor_->processIntegerLiteral(integerLiteral);
  } else if (const auto *floatingLiteral =
                 llvm::dyn_cast<clang::FloatingLiteral>(expr)) {
    expr_processor_->processFloatingLiteral(floatingLiteral);
  } else if (const auto *characterLiteral =
                 llvm::dyn_cast<clang::CharacterLiteral>(expr)) {
    expr_processor_->processCharacterLiteral(characterLiteral);
  } else if (const auto *boolLiteral =
                 llvm::dyn_cast<clang::CXXBoolLiteralExpr>(expr)) {
    expr_processor_->processBoolLiteral(boolLiteral);
  } else if (const auto *declRefExpr =
                 llvm::dyn_cast<clang::DeclRefExpr>(expr)) {
    expr_processor_->processDeclRef(
        const_cast<clang::DeclRefExpr *>(declRefExpr));
  } else {
    return -1;
  }

  if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
    return *cachedId;

  return -1;
}

void TemplateProcessor::recordClassTemplateTypeArguments(
    int typeId, const clang::TemplateArgumentList &templateArgs) {
  if (typeId == -1)
    return;

  for (unsigned index = 0; index < templateArgs.size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs[index];
    if (arg.getKind() != clang::TemplateArgument::Type)
      continue;

    int argTypeId =
        this->resolveTemplateArgumentTypeId(arg.getAsType());
    if (argTypeId == -1)
      continue;

    if (!this->shouldInsertClassTemplateArgument(
            typeId, static_cast<int>(index), argTypeId))
      continue;

    DbModel::ClassTemplateArgument templateArgument = {
        typeId, static_cast<int>(index), argTypeId};
    STG.insertClassObj(templateArgument);
  }
}

void TemplateProcessor::recordClassTemplateArgumentValues(
    int typeId, const clang::ASTTemplateArgumentListInfo *templateArgs) {
  if (typeId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < templateArgs->getNumTemplateArgs();
       ++index) {
    const clang::TemplateArgumentLoc &argLoc = (*templateArgs)[index];
    const clang::Expr *sourceExpr =
        this->getTemplateArgumentSourceExpr(argLoc);
    int exprId =
        this->resolveTemplateArgumentExprId(sourceExpr);
    if (exprId == -1)
      continue;

    if (!this->shouldInsertClassTemplateArgumentValue(
            typeId, static_cast<int>(index), exprId))
      continue;

    DbModel::ClassTemplateArgumentValue templateArgumentValue = {
        typeId, static_cast<int>(index), exprId};
    STG.insertClassObj(templateArgumentValue);
  }
}

void TemplateProcessor::recordClassTemplateArgumentValues(
    int typeId, clang::TemplateSpecializationTypeLoc templateArgs) {
  if (typeId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < templateArgs.getNumArgs(); ++index) {
    const clang::TemplateArgumentLoc &argLoc = templateArgs.getArgLoc(index);
    const clang::Expr *sourceExpr =
        this->getTemplateArgumentSourceExpr(argLoc);
    int exprId =
        this->resolveTemplateArgumentExprId(sourceExpr);
    if (exprId == -1)
      continue;

    if (!this->shouldInsertClassTemplateArgumentValue(
            typeId, static_cast<int>(index), exprId))
      continue;

    DbModel::ClassTemplateArgumentValue templateArgumentValue = {
        typeId, static_cast<int>(index), exprId};
    STG.insertClassObj(templateArgumentValue);
  }
}

void TemplateProcessor::recordTemplateTemplateArguments(
    int typeId, const clang::TemplateArgumentList &templateArgs) {
  if (typeId == -1)
    return;

  for (unsigned index = 0; index < templateArgs.size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs[index];
    int argTypeId =
        this->resolveTemplateTemplateArgumentTypeId(arg);
    if (argTypeId == -1)
      continue;

    if (!this->shouldInsertTemplateTemplateArgument(
            typeId, static_cast<int>(index), argTypeId))
      continue;

    DbModel::TemplateTemplateArgument templateArgument = {
        typeId, static_cast<int>(index), argTypeId};
    STG.insertClassObj(templateArgument);
  }
}

void TemplateProcessor::recordTemplateTemplateInstantiations(
    const clang::ClassTemplateDecl *classTemplateDecl,
    const clang::TemplateArgumentList &templateArgs) {
  if (!classTemplateDecl || !type_processor_ || !ast_context_)
    return;

  const clang::TemplateParameterList *params =
      classTemplateDecl->getTemplateParameters();
  if (!params)
    return;

  unsigned count = templateArgs.size();
  if (params->size() < count)
    count = params->size();

  for (unsigned index = 0; index < count; ++index) {
    const auto *param = llvm::dyn_cast_or_null<clang::TemplateTemplateParmDecl>(
        params->getParam(index));
    if (!param || param->isParameterPack())
      continue;

    const clang::TemplateArgument &arg = templateArgs[index];
    const clang::TemplateDecl *templateDecl =
        arg.getKind() == clang::TemplateArgument::Template
            ? arg.getAsTemplate().getAsTemplateDecl()
            : nullptr;
    if (!llvm::isa_and_nonnull<clang::ClassTemplateDecl>(templateDecl))
      continue;

    int paramId = this->resolveTemplateTemplateParmId(param);
    int argTypeId =
        this->resolveTemplateTemplateArgumentTypeId(arg);
    if (paramId == -1 || argTypeId == -1)
      continue;

    if (!this->shouldInsertTemplateTemplateInstantiation(argTypeId,
                                                         paramId))
      continue;

    DbModel::TemplateTemplateInstantiation instantiation = {argTypeId,
                                                            paramId};
    STG.insertClassObj(instantiation);
  }
}

void TemplateProcessor::recordFunctionTemplateTypeArguments(
    int functionId, const clang::TemplateArgumentList *templateArgs) {
  if (functionId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < templateArgs->size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs->get(index);
    if (arg.getKind() != clang::TemplateArgument::Type)
      continue;

    int argTypeId =
        this->resolveTemplateArgumentTypeId(arg.getAsType());
    if (argTypeId == -1)
      continue;

    if (!this->shouldInsertFunctionTemplateArgument(
            functionId, static_cast<int>(index), argTypeId))
      continue;

    DbModel::FunctionTemplateArgument templateArgument = {
        functionId, static_cast<int>(index), argTypeId};
    STG.insertClassObj(templateArgument);
  }
}

void TemplateProcessor::recordFunctionTemplateArgumentValues(
    int functionId, const clang::ASTTemplateArgumentListInfo *templateArgs) {
  if (functionId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < templateArgs->getNumTemplateArgs();
       ++index) {
    const clang::TemplateArgumentLoc &argLoc = (*templateArgs)[index];
    const clang::Expr *sourceExpr =
        this->getTemplateArgumentSourceExpr(argLoc);
    int exprId =
        this->resolveTemplateArgumentExprId(sourceExpr);
    if (exprId == -1)
      continue;

    if (!this->shouldInsertFunctionTemplateArgumentValue(
            functionId, static_cast<int>(index), exprId))
      continue;

    DbModel::FunctionTemplateArgumentValue templateArgumentValue = {
        functionId, static_cast<int>(index), exprId};
    STG.insertClassObj(templateArgumentValue);
  }
}

void TemplateProcessor::recordFunctionTemplateArgumentValues(
    int functionId, const clang::TemplateArgumentLoc *templateArgs,
    unsigned numTemplateArgs) {
  if (functionId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < numTemplateArgs; ++index) {
    const clang::TemplateArgumentLoc &argLoc = templateArgs[index];
    const clang::Expr *sourceExpr =
        this->getTemplateArgumentSourceExpr(argLoc);
    int exprId =
        this->resolveTemplateArgumentExprId(sourceExpr);
    if (exprId == -1)
      continue;

    if (!this->shouldInsertFunctionTemplateArgumentValue(
            functionId, static_cast<int>(index), exprId))
      continue;

    DbModel::FunctionTemplateArgumentValue templateArgumentValue = {
        functionId, static_cast<int>(index), exprId};
    STG.insertClassObj(templateArgumentValue);
  }
}

void TemplateProcessor::recordVariableTemplateTypeArguments(
    int variableId, const clang::TemplateArgumentList &templateArgs) {
  if (variableId == -1)
    return;

  for (unsigned index = 0; index < templateArgs.size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs[index];
    if (arg.getKind() != clang::TemplateArgument::Type)
      continue;

    int argTypeId =
        this->resolveTemplateArgumentTypeId(arg.getAsType());
    if (argTypeId == -1)
      continue;

    if (!this->shouldInsertVariableTemplateArgument(
            variableId, static_cast<int>(index), argTypeId))
      continue;

    DbModel::VariableTemplateArgument templateArgument = {
        variableId, static_cast<int>(index), argTypeId};
    STG.insertClassObj(templateArgument);
  }
}

void TemplateProcessor::recordVariableTemplateArgumentValues(
    int variableId, const clang::ASTTemplateArgumentListInfo *templateArgs) {
  if (variableId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < templateArgs->getNumTemplateArgs();
       ++index) {
    const clang::TemplateArgumentLoc &argLoc = (*templateArgs)[index];
    const clang::Expr *sourceExpr =
        this->getTemplateArgumentSourceExpr(argLoc);
    int exprId =
        this->resolveTemplateArgumentExprId(sourceExpr);
    if (exprId == -1)
      continue;

    if (!this->shouldInsertVariableTemplateArgumentValue(
            variableId, static_cast<int>(index), exprId))
      continue;

    DbModel::VariableTemplateArgumentValue templateArgumentValue = {
        variableId, static_cast<int>(index), exprId};
    STG.insertClassObj(templateArgumentValue);
  }
}

void TemplateProcessor::recordConceptTemplateTypeArguments(
    int conceptId, llvm::ArrayRef<clang::TemplateArgument> templateArgs) {
  if (conceptId == -1)
    return;

  for (unsigned index = 0; index < templateArgs.size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs[index];
    if (arg.getKind() != clang::TemplateArgument::Type)
      continue;

    int argTypeId =
        this->resolveTemplateArgumentTypeId(arg.getAsType());
    if (argTypeId == -1)
      continue;

    if (!this->shouldInsertConceptTemplateArgument(
            conceptId, static_cast<int>(index), argTypeId))
      continue;

    DbModel::ConceptTemplateArgument templateArgument = {
        conceptId, static_cast<int>(index), argTypeId};
    STG.insertClassObj(templateArgument);
  }
}

void TemplateProcessor::recordConceptTemplateArgumentValues(
    int conceptId, const clang::ConceptSpecializationExpr *expr) {
  if (conceptId == -1 || !expr || !expr_processor_ || !ast_context_)
    return;

  llvm::ArrayRef<clang::TemplateArgument> templateArgs =
      expr->getTemplateArguments();

  const clang::ASTTemplateArgumentListInfo *argsAsWritten =
      expr->getTemplateArgsAsWritten();

  for (unsigned index = 0; index < templateArgs.size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs[index];
    const clang::Expr *sourceExpr = nullptr;

    // Prefer source expression from args-as-written (with location info)
    if (argsAsWritten && index < argsAsWritten->getNumTemplateArgs()) {
      const clang::TemplateArgumentLoc &argLoc = (*argsAsWritten)[index];
      sourceExpr = this->getTemplateArgumentSourceExpr(argLoc);
    }

    // Fall back: Expression kind directly stores the source expr
    if (!sourceExpr && arg.getKind() == clang::TemplateArgument::Expression) {
      sourceExpr = arg.getAsExpr();
    }

    // Defer: no source expression available (e.g., Integral without source loc,
    // SubstNonTypeTemplateParmExpr, dependent, pack, null, etc.)
    if (!sourceExpr)
      continue;

    // Check for SubstNonTypeTemplateParmExpr — defer
    if (llvm::isa<clang::SubstNonTypeTemplateParmExpr>(sourceExpr))
      continue;

    int exprId =
        this->resolveTemplateArgumentExprId(sourceExpr);
    if (exprId == -1)
      continue;

    if (!this->shouldInsertConceptTemplateArgumentValue(
            conceptId, static_cast<int>(index), exprId))
      continue;

    DbModel::ConceptTemplateArgumentValue row = {conceptId,
                                                  static_cast<int>(index),
                                                  exprId};
    STG.insertClassObj(row);
  }
}

void TemplateProcessor::recordTemplateTypeConstraint(
    const clang::TemplateTypeParmDecl *decl) {
  if (!decl || !expr_processor_ || !ast_context_)
    return;

  const clang::TypeConstraint *typeConstraint = decl->getTypeConstraint();
  if (!typeConstraint)
    return;

  const clang::Expr *constraintExpr =
      typeConstraint->getImmediatelyDeclaredConstraint();
  const auto *conceptExpr =
      llvm::dyn_cast_or_null<clang::ConceptSpecializationExpr>(constraintExpr);
  if (!conceptExpr)
    return;

  KeyType templateParamKey = KeyGen::Type::makeKey(decl, ast_context_);
  int templateParamId = SEARCH_TYPE_CACHE(templateParamKey).value_or(-1);
  if (templateParamId == -1)
    return;

  int conceptId = this->resolveConceptSpecializationId(conceptExpr,
                                                       ast_context_);
  if (conceptId == -1)
    return;

  int constraintExprId =
      expr_processor_->processConceptSpecializationExpr(conceptExpr, conceptId);
  if (constraintExprId == -1)
    return;

  if (this->shouldInsertTypeTemplateTypeConstraint(
          templateParamId, constraintExprId)) {
    DbModel::TypeTemplateTypeConstraint row = {templateParamId,
                                               constraintExprId};
    STG.insertClassObj(row);
  }

  if (this->shouldInsertIsTypeConstraint(conceptId)) {
    DbModel::IsTypeConstraint row = {conceptId};
    STG.insertClassObj(row);
  }
}

void TemplateProcessor::processFunctionTemplateSpecialization(
    const clang::FunctionDecl *decl, int functionId) {
  if (decl && decl->isFunctionTemplateSpecialization()) {
    KeyType functionKey = KeyGen::Function::makeKey(decl, ast_context_);
    int specializationId = functionId;
    if (auto cachedId = SEARCH_FUNCTION_CACHE(functionKey))
      specializationId = *cachedId;
    if (specializationId == -1)
      return;

    if (const clang::FunctionTemplateDecl *primaryTemplate =
            decl->getPrimaryTemplate()) {
      const clang::FunctionDecl *templatedDecl =
          primaryTemplate->getTemplatedDecl();
      KeyType templateKey = KeyGen::Function::makeKey(templatedDecl,
                                                      ast_context_);

      if (auto cachedId = SEARCH_FUNCTION_CACHE(templateKey)) {
        if (this->shouldInsertFunctionInstantiation(
                specializationId, *cachedId)) {
          DbModel::FunctionInstantiation instantiation = {
              specializationId, *cachedId};
          STG.insertClassObj(instantiation);
        }
      } else {
        PendingUpdate update{
            templateKey, CacheType::FUNCTION,
            [specializationId,
             templateProcessor = this](int resolvedId) {
              if (!templateProcessor->shouldInsertFunctionInstantiation(
                      specializationId, resolvedId))
                return;

              DbModel::FunctionInstantiation instantiation = {
                  specializationId, resolvedId};
              STG.insertClassObj(instantiation);
            }};
        DependencyManager::instance().addDependency(update);
      }
    }

    this->recordFunctionTemplateTypeArguments(
        specializationId, decl->getTemplateSpecializationArgs());
    this->recordFunctionTemplateArgumentValues(
        specializationId, decl->getTemplateSpecializationArgsAsWritten());
  }
}

void TemplateProcessor::processVarTemplateSpecialization(
    const clang::VarTemplateSpecializationDecl *decl, int) {
  if (const auto *specialization =
          llvm::dyn_cast_or_null<clang::VarTemplateSpecializationDecl>(decl)) {
    int variableId =
        this->resolveVariableEntityId(specialization);
    if (variableId == -1)
      return;

    const clang::VarTemplateDecl *primaryTemplate =
        specialization->getSpecializedTemplate();
    if (primaryTemplate && primaryTemplate->getTemplatedDecl()) {
      int templateId = this->resolveVariableEntityId(
          primaryTemplate->getTemplatedDecl());
      if (templateId != -1 &&
          this->shouldInsertVariableInstantiation(variableId,
                                                  templateId)) {
        DbModel::VariableInstantiation instantiation = {variableId,
                                                        templateId};
        STG.insertClassObj(instantiation);
      }
    }

    this->recordVariableTemplateTypeArguments(
        variableId, specialization->getTemplateInstantiationArgs());
    this->recordVariableTemplateArgumentValues(
        variableId, specialization->getTemplateArgsAsWritten());
  }
}

void TemplateProcessor::processClassTemplateSpecialization(
    const clang::ClassTemplateSpecializationDecl *decl) {
  if (!decl)
    return;

  const clang::ClassTemplateDecl *classTemplateDecl = nullptr;
  auto instantiatedFrom = decl->getInstantiatedFrom();
  if (!instantiatedFrom.isNull())
    classTemplateDecl = instantiatedFrom.dyn_cast<clang::ClassTemplateDecl *>();
  if (!classTemplateDecl)
    classTemplateDecl = decl->getSpecializedTemplate();
  if (!classTemplateDecl)
    return;

  const clang::CXXRecordDecl *templatedDecl =
      classTemplateDecl->getTemplatedDecl();
  if (!templatedDecl)
    return;

  KeyType specializationKey = KeyGen::Type::makeKey(decl, ast_context_);
  if (!SEARCH_TYPE_CACHE(specializationKey))
    type_processor_->processRecordDeclType(decl);

  KeyType templateKey = KeyGen::Type::makeKey(templatedDecl, ast_context_);
  if (!SEARCH_TYPE_CACHE(templateKey))
    type_processor_->processRecordDeclType(templatedDecl);

  int specializationId = -1;
  int templateId = -1;

  if (auto cachedId = SEARCH_TYPE_CACHE(specializationKey))
    specializationId = *cachedId;
  if (auto cachedId = SEARCH_TYPE_CACHE(templateKey))
    templateId = *cachedId;

  if (specializationId != -1 && templateId != -1) {
    if (this->shouldInsertClassInstantiation(specializationId,
                                             templateId)) {
      DbModel::ClassInstantiation instantiation = {specializationId,
                                                   templateId};
      STG.insertClassObj(instantiation);
    }
  }

  this->recordClassTemplateTypeArguments(
      specializationId, decl->getTemplateInstantiationArgs());
  this->recordTemplateTemplateArguments(
      specializationId, decl->getTemplateInstantiationArgs());
  this->recordTemplateTemplateInstantiations(
      classTemplateDecl, decl->getTemplateInstantiationArgs());
  this->recordClassTemplateArgumentValues(
      specializationId, decl->getTemplateArgsAsWritten());
}

void TemplateProcessor::processFriendDecl(const clang::FriendDecl *decl) {
  if (!decl)
    return;

  LocIdPair *locIdPair = SrcLocRecorder::processDefault(decl, ast_context_);
  const int friendDeclId = GENID(FriendDecl);
  int typeId = -1;
  int declId = -1;

  auto resolveRecordType = [&](const clang::RecordType *recordType) -> int {
    if (!recordType || !recordType->getDecl())
      return -1;

    type_processor_->processRecordType(recordType);
    KeyType typeKey = KeyGen::Type::makeKey(recordType->getDecl(),
                                            ast_context_);
    if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
      return *cachedId;

    return -1;
  };

  auto resolveFriendType = [&](clang::QualType friendType) -> int {
    if (friendType.isNull())
      return -1;

    if (const auto *recordType = friendType->getAs<clang::RecordType>())
      return resolveRecordType(recordType);

    return type_processor_->processType(friendType.getTypePtr());
  };

  if (clang::TypeSourceInfo *friendTypeInfo = decl->getFriendType()) {
    typeId = resolveFriendType(friendTypeInfo->getType());
  } else if (clang::NamedDecl *friendNamedDecl = decl->getFriendDecl()) {
    if (const auto *functionDecl =
            llvm::dyn_cast<clang::FunctionDecl>(friendNamedDecl)) {
      KeyType functionKey = KeyGen::Function::makeKey(functionDecl,
                                                      ast_context_);
      if (auto cachedId = SEARCH_FUNCTION_CACHE(functionKey)) {
        declId = *cachedId;
      } else {
        PendingUpdate update{
            functionKey, CacheType::FUNCTION,
            [friendDeclId, typeId, location = locIdPair->spec_id](
                int resolvedId) {
              DbModel::FriendDecl updatedRecord = {friendDeclId, typeId,
                                                   resolvedId, location};
              STG.insertClassObj(updatedRecord);
            }};
        DependencyManager::instance().addDependency(update);
      }
    } else if (const auto *typeDecl =
                   llvm::dyn_cast<clang::TypeDecl>(friendNamedDecl)) {
      if (const auto *recordDecl =
              llvm::dyn_cast<clang::RecordDecl>(typeDecl)) {
        if (const auto *recordType =
                llvm::dyn_cast_or_null<clang::RecordType>(
                    recordDecl->getTypeForDecl()))
          typeId = resolveRecordType(recordType);
      }
    }
  }

  DbModel::FriendDecl friendDeclModel = {friendDeclId, typeId, declId,
                                         locIdPair->spec_id};
  STG.insertClassObj(friendDeclModel);
}

void TemplateProcessor::processConceptSpecialization(
    const clang::ConceptSpecializationExpr *expr) {
  if (!expr)
    return;

  int templateId = this->resolveConceptTemplateId(
      expr->getNamedConcept(), ast_context_);
  if (templateId == -1)
    return;

  int conceptId = this->resolveConceptSpecializationId(expr,
                                                       ast_context_);
  if (conceptId == -1)
    return;

  expr_processor_->processConceptSpecializationExpr(expr, conceptId);

  if (this->shouldInsertConceptInstantiation(conceptId,
                                             templateId)) {
    DbModel::ConceptInstantiation instantiation = {conceptId, templateId};
    STG.insertClassObj(instantiation);
  }

  this->recordConceptTemplateTypeArguments(
      conceptId, expr->getTemplateArguments());
  this->recordConceptTemplateArgumentValues(conceptId, expr);
}
