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

namespace {

std::unordered_set<std::string> classInstantiationDedup;
std::unordered_set<std::string> classTemplateArgumentDedup;
std::unordered_set<std::string> classTemplateArgumentValueDedup;
std::unordered_set<std::string> functionInstantiationDedup;
std::unordered_set<std::string> functionTemplateArgumentDedup;
std::unordered_set<std::string> functionTemplateArgumentValueDedup;
std::unordered_set<std::string> variableTemplateDedup;
std::unordered_set<std::string> variableInstantiationDedup;
std::unordered_set<std::string> variableTemplateArgumentDedup;
std::unordered_set<std::string> variableTemplateArgumentValueDedup;
std::unordered_set<std::string> templateTemplateArgumentDedup;
std::unordered_set<std::string> conceptInstantiationDedup;
std::unordered_set<std::string> conceptTemplateArgumentDedup;
std::unordered_map<std::string, int> conceptTemplateIds;
std::unordered_map<std::string, int> conceptSpecializationIds;

struct ConceptSpecializationId {
  int id;
};

std::string makePairKey(int first, int second) {
  return std::to_string(first) + ":" + std::to_string(second);
}

std::string makeTripleKey(int first, int second, int third) {
  return std::to_string(first) + ":" + std::to_string(second) + ":" +
         std::to_string(third);
}

bool shouldInsertClassInstantiation(int to, int from) {
  return classInstantiationDedup.insert(makePairKey(to, from)).second;
}

bool shouldInsertClassTemplateArgument(int typeId, int index, int argType) {
  return classTemplateArgumentDedup
      .insert(makeTripleKey(typeId, index, argType))
      .second;
}

bool shouldInsertClassTemplateArgumentValue(int typeId, int index,
                                            int argValue) {
  return classTemplateArgumentValueDedup
      .insert(makeTripleKey(typeId, index, argValue))
      .second;
}

bool shouldInsertFunctionInstantiation(int to, int from) {
  return functionInstantiationDedup.insert(makePairKey(to, from)).second;
}

bool shouldInsertFunctionTemplateArgument(int functionId, int index,
                                          int argType) {
  return functionTemplateArgumentDedup
      .insert(makeTripleKey(functionId, index, argType))
      .second;
}

bool shouldInsertFunctionTemplateArgumentValue(int functionId, int index,
                                               int argValue) {
  return functionTemplateArgumentValueDedup
      .insert(makeTripleKey(functionId, index, argValue))
      .second;
}

bool shouldInsertVariableTemplate(int variableId) {
  return variableTemplateDedup.insert(std::to_string(variableId)).second;
}

bool shouldInsertVariableInstantiation(int to, int from) {
  return variableInstantiationDedup
      .insert(makePairKey(to, from))
      .second;
}

bool shouldInsertVariableTemplateArgument(int variableId, int index,
                                          int argType) {
  return variableTemplateArgumentDedup
      .insert(makeTripleKey(variableId, index, argType))
      .second;
}

bool shouldInsertVariableTemplateArgumentValue(int variableId, int index,
                                               int argValue) {
  return variableTemplateArgumentValueDedup
      .insert(makeTripleKey(variableId, index, argValue))
      .second;
}

bool shouldInsertTemplateTemplateArgument(int typeId, int index, int argType) {
  return templateTemplateArgumentDedup
      .insert(makeTripleKey(typeId, index, argType))
      .second;
}

bool shouldInsertConceptInstantiation(int conceptId, int templateId) {
  return conceptInstantiationDedup.insert(makePairKey(conceptId, templateId))
      .second;
}

bool shouldInsertConceptTemplateArgument(int conceptId, int index,
                                         int argType) {
  return conceptTemplateArgumentDedup
      .insert(makeTripleKey(conceptId, index, argType))
      .second;
}

int resolveVariableEntityId(const clang::VarDecl *decl,
                            VariableProcessor *variableProcessor,
                            clang::ASTContext *context) {
  if (!decl || !variableProcessor || !context)
    return -1;

  KeyType variableKey = KeyGen::Var::makeKey(decl, context);
  if (auto cachedId = SEARCH_VARIABLE_CACHE(variableKey))
    return *cachedId;

  variableProcessor->processVarDecl(decl);
  if (auto cachedId = SEARCH_VARIABLE_CACHE(variableKey))
    return *cachedId;

  return -1;
}

int resolveTemplateArgumentTypeId(clang::QualType argType,
                                  TypeProcessor *typeProcessor,
                                  clang::ASTContext *context) {
  if (argType.isNull() || !typeProcessor || !context)
    return -1;

  KeyType typeKey = KeyGen::Type::makeKey(argType, context);
  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    return *cachedId;

  if (const auto *builtinType = argType->getAs<clang::BuiltinType>())
    return typeProcessor->processBuiltinType(builtinType, context);

  if (const auto *recordType = argType->getAs<clang::RecordType>()) {
    typeProcessor->processRecordType(recordType);
    if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
      return *cachedId;
  }

  int typeId = typeProcessor->processType(argType.getTypePtr());
  if (typeId != -1)
    return typeId;

  if (auto cachedId = SEARCH_TYPE_CACHE(typeKey))
    return *cachedId;

  return -1;
}

int resolveTemplateTemplateArgumentTypeId(const clang::TemplateArgument &arg,
                                          TypeProcessor *typeProcessor,
                                          clang::ASTContext *context) {
  if (arg.getKind() != clang::TemplateArgument::Template || !typeProcessor ||
      !context)
    return -1;

  const clang::TemplateDecl *templateDecl =
      arg.getAsTemplate().getAsTemplateDecl();
  const auto *classTemplateDecl =
      llvm::dyn_cast_or_null<clang::ClassTemplateDecl>(templateDecl);
  if (!classTemplateDecl || !classTemplateDecl->getTemplatedDecl())
    return -1;

  const clang::CXXRecordDecl *templatedDecl =
      classTemplateDecl->getTemplatedDecl();
  KeyType templateTypeKey = KeyGen::Type::makeKey(templatedDecl, context);
  if (auto cachedId = SEARCH_TYPE_CACHE(templateTypeKey))
    return *cachedId;

  int templateTypeId = typeProcessor->processRecordDeclType(templatedDecl);
  if (templateTypeId != -1)
    return templateTypeId;

  if (auto cachedId = SEARCH_TYPE_CACHE(templateTypeKey))
    return *cachedId;

  return -1;
}

std::string makeConceptTemplateKey(const clang::ConceptDecl *decl,
                                   clang::ASTContext *context) {
  if (!decl || !context)
    return "";

  const clang::ConceptDecl *canonicalDecl = decl->getCanonicalDecl();
  return KeyGen::Type::makeKey(canonicalDecl, context);
}

int resolveConceptTemplateId(const clang::ConceptDecl *decl,
                             clang::ASTContext *context) {
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

std::string makeTemplateArgumentListKey(llvm::ArrayRef<clang::TemplateArgument> args,
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

std::string makeConceptSpecializationKey(
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

int resolveConceptSpecializationId(
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

const clang::Expr *
getTemplateArgumentSourceExpr(const clang::TemplateArgumentLoc &argLoc) {
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

int resolveTemplateArgumentExprId(const clang::Expr *sourceExpr,
                                  ExprProcessor *exprProcessor,
                                  clang::ASTContext *context) {
  if (!sourceExpr || !exprProcessor || !context)
    return -1;

  const clang::Expr *expr = sourceExpr->IgnoreParenImpCasts();
  KeyType exprKey = KeyGen::Expr_::makeKey(expr, context);
  if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
    return *cachedId;

  if (const auto *integerLiteral =
          llvm::dyn_cast<clang::IntegerLiteral>(expr)) {
    exprProcessor->processIntegerLiteral(integerLiteral);
  } else if (const auto *floatingLiteral =
                 llvm::dyn_cast<clang::FloatingLiteral>(expr)) {
    exprProcessor->processFloatingLiteral(floatingLiteral);
  } else if (const auto *characterLiteral =
                 llvm::dyn_cast<clang::CharacterLiteral>(expr)) {
    exprProcessor->processCharacterLiteral(characterLiteral);
  } else if (const auto *boolLiteral =
                 llvm::dyn_cast<clang::CXXBoolLiteralExpr>(expr)) {
    exprProcessor->processBoolLiteral(boolLiteral);
  } else {
    return -1;
  }

  if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
    return *cachedId;

  return -1;
}

void recordClassTemplateTypeArguments(
    int typeId, const clang::TemplateArgumentList &templateArgs,
    TypeProcessor *typeProcessor, clang::ASTContext *context) {
  if (typeId == -1)
    return;

  for (unsigned index = 0; index < templateArgs.size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs[index];
    if (arg.getKind() != clang::TemplateArgument::Type)
      continue;

    int argTypeId =
        resolveTemplateArgumentTypeId(arg.getAsType(), typeProcessor, context);
    if (argTypeId == -1)
      continue;

    if (!shouldInsertClassTemplateArgument(typeId, static_cast<int>(index),
                                           argTypeId))
      continue;

    DbModel::ClassTemplateArgument templateArgument = {
        typeId, static_cast<int>(index), argTypeId};
    STG.insertClassObj(templateArgument);
  }
}

void recordClassTemplateArgumentValues(
    int typeId, const clang::ASTTemplateArgumentListInfo *templateArgs,
    ExprProcessor *exprProcessor, clang::ASTContext *context) {
  if (typeId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < templateArgs->getNumTemplateArgs();
       ++index) {
    const clang::TemplateArgumentLoc &argLoc = (*templateArgs)[index];
    const clang::Expr *sourceExpr = getTemplateArgumentSourceExpr(argLoc);
    int exprId =
        resolveTemplateArgumentExprId(sourceExpr, exprProcessor, context);
    if (exprId == -1)
      continue;

    if (!shouldInsertClassTemplateArgumentValue(typeId,
                                                static_cast<int>(index),
                                                exprId))
      continue;

    DbModel::ClassTemplateArgumentValue templateArgumentValue = {
        typeId, static_cast<int>(index), exprId};
    STG.insertClassObj(templateArgumentValue);
  }
}

void recordClassTemplateArgumentValues(
    int typeId, clang::TemplateSpecializationTypeLoc templateArgs,
    ExprProcessor *exprProcessor, clang::ASTContext *context) {
  if (typeId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < templateArgs.getNumArgs(); ++index) {
    const clang::TemplateArgumentLoc &argLoc = templateArgs.getArgLoc(index);
    const clang::Expr *sourceExpr = getTemplateArgumentSourceExpr(argLoc);
    int exprId =
        resolveTemplateArgumentExprId(sourceExpr, exprProcessor, context);
    if (exprId == -1)
      continue;

    if (!shouldInsertClassTemplateArgumentValue(typeId,
                                                static_cast<int>(index),
                                                exprId))
      continue;

    DbModel::ClassTemplateArgumentValue templateArgumentValue = {
        typeId, static_cast<int>(index), exprId};
    STG.insertClassObj(templateArgumentValue);
  }
}

void recordTemplateTemplateArguments(
    int typeId, const clang::TemplateArgumentList &templateArgs,
    TypeProcessor *typeProcessor, clang::ASTContext *context) {
  if (typeId == -1)
    return;

  for (unsigned index = 0; index < templateArgs.size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs[index];
    int argTypeId =
        resolveTemplateTemplateArgumentTypeId(arg, typeProcessor, context);
    if (argTypeId == -1)
      continue;

    if (!shouldInsertTemplateTemplateArgument(
            typeId, static_cast<int>(index), argTypeId))
      continue;

    DbModel::TemplateTemplateArgument templateArgument = {
        typeId, static_cast<int>(index), argTypeId};
    STG.insertClassObj(templateArgument);
  }
}

void recordFunctionTemplateTypeArguments(
    int functionId, const clang::TemplateArgumentList *templateArgs,
    TypeProcessor *typeProcessor, clang::ASTContext *context) {
  if (functionId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < templateArgs->size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs->get(index);
    if (arg.getKind() != clang::TemplateArgument::Type)
      continue;

    int argTypeId =
        resolveTemplateArgumentTypeId(arg.getAsType(), typeProcessor, context);
    if (argTypeId == -1)
      continue;

    if (!shouldInsertFunctionTemplateArgument(
            functionId, static_cast<int>(index), argTypeId))
      continue;

    DbModel::FunctionTemplateArgument templateArgument = {
        functionId, static_cast<int>(index), argTypeId};
    STG.insertClassObj(templateArgument);
  }
}

void recordFunctionTemplateArgumentValues(
    int functionId, const clang::ASTTemplateArgumentListInfo *templateArgs,
    ExprProcessor *exprProcessor, clang::ASTContext *context) {
  if (functionId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < templateArgs->getNumTemplateArgs();
       ++index) {
    const clang::TemplateArgumentLoc &argLoc = (*templateArgs)[index];
    const clang::Expr *sourceExpr = getTemplateArgumentSourceExpr(argLoc);
    int exprId =
        resolveTemplateArgumentExprId(sourceExpr, exprProcessor, context);
    if (exprId == -1)
      continue;

    if (!shouldInsertFunctionTemplateArgumentValue(functionId,
                                                   static_cast<int>(index),
                                                   exprId))
      continue;

    DbModel::FunctionTemplateArgumentValue templateArgumentValue = {
        functionId, static_cast<int>(index), exprId};
    STG.insertClassObj(templateArgumentValue);
  }
}

void recordFunctionTemplateArgumentValues(
    int functionId, const clang::TemplateArgumentLoc *templateArgs,
    unsigned numTemplateArgs, ExprProcessor *exprProcessor,
    clang::ASTContext *context) {
  if (functionId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < numTemplateArgs; ++index) {
    const clang::TemplateArgumentLoc &argLoc = templateArgs[index];
    const clang::Expr *sourceExpr = getTemplateArgumentSourceExpr(argLoc);
    int exprId =
        resolveTemplateArgumentExprId(sourceExpr, exprProcessor, context);
    if (exprId == -1)
      continue;

    if (!shouldInsertFunctionTemplateArgumentValue(functionId,
                                                   static_cast<int>(index),
                                                   exprId))
      continue;

    DbModel::FunctionTemplateArgumentValue templateArgumentValue = {
        functionId, static_cast<int>(index), exprId};
    STG.insertClassObj(templateArgumentValue);
  }
}

void recordVariableTemplateTypeArguments(
    int variableId, const clang::TemplateArgumentList &templateArgs,
    TypeProcessor *typeProcessor, clang::ASTContext *context) {
  if (variableId == -1)
    return;

  for (unsigned index = 0; index < templateArgs.size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs[index];
    if (arg.getKind() != clang::TemplateArgument::Type)
      continue;

    int argTypeId =
        resolveTemplateArgumentTypeId(arg.getAsType(), typeProcessor, context);
    if (argTypeId == -1)
      continue;

    if (!shouldInsertVariableTemplateArgument(
            variableId, static_cast<int>(index), argTypeId))
      continue;

    DbModel::VariableTemplateArgument templateArgument = {
        variableId, static_cast<int>(index), argTypeId};
    STG.insertClassObj(templateArgument);
  }
}

void recordVariableTemplateArgumentValues(
    int variableId, const clang::ASTTemplateArgumentListInfo *templateArgs,
    ExprProcessor *exprProcessor, clang::ASTContext *context) {
  if (variableId == -1 || !templateArgs)
    return;

  for (unsigned index = 0; index < templateArgs->getNumTemplateArgs();
       ++index) {
    const clang::TemplateArgumentLoc &argLoc = (*templateArgs)[index];
    const clang::Expr *sourceExpr = getTemplateArgumentSourceExpr(argLoc);
    int exprId =
        resolveTemplateArgumentExprId(sourceExpr, exprProcessor, context);
    if (exprId == -1)
      continue;

    if (!shouldInsertVariableTemplateArgumentValue(
            variableId, static_cast<int>(index), exprId))
      continue;

    DbModel::VariableTemplateArgumentValue templateArgumentValue = {
        variableId, static_cast<int>(index), exprId};
    STG.insertClassObj(templateArgumentValue);
  }
}

void recordConceptTemplateTypeArguments(
    int conceptId, llvm::ArrayRef<clang::TemplateArgument> templateArgs,
    TypeProcessor *typeProcessor, clang::ASTContext *context) {
  if (conceptId == -1)
    return;

  for (unsigned index = 0; index < templateArgs.size(); ++index) {
    const clang::TemplateArgument &arg = templateArgs[index];
    if (arg.getKind() != clang::TemplateArgument::Type)
      continue;

    int argTypeId =
        resolveTemplateArgumentTypeId(arg.getAsType(), typeProcessor, context);
    if (argTypeId == -1)
      continue;

    if (!shouldInsertConceptTemplateArgument(conceptId,
                                             static_cast<int>(index),
                                             argTypeId))
      continue;

    DbModel::ConceptTemplateArgument templateArgument = {
        conceptId, static_cast<int>(index), argTypeId};
    STG.insertClassObj(templateArgument);
  }
}

} // namespace

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

  if (decl && decl->isFunctionTemplateSpecialization()) {
    KeyType functionKey = KeyGen::Function::makeKey(decl, context_);
    int specializationId = func_id;
    if (auto cachedId = SEARCH_FUNCTION_CACHE(functionKey))
      specializationId = *cachedId;
    if (specializationId == -1)
      return true;

    if (const clang::FunctionTemplateDecl *primaryTemplate =
            decl->getPrimaryTemplate()) {
      const clang::FunctionDecl *templatedDecl =
          primaryTemplate->getTemplatedDecl();
      KeyType templateKey = KeyGen::Function::makeKey(templatedDecl, context_);

      if (auto cachedId = SEARCH_FUNCTION_CACHE(templateKey)) {
        if (shouldInsertFunctionInstantiation(specializationId, *cachedId)) {
          DbModel::FunctionInstantiation instantiation = {
              specializationId, *cachedId};
          STG.insertClassObj(instantiation);
        }
      } else {
        PendingUpdate update{
            templateKey, CacheType::FUNCTION,
            [specializationId](int resolvedId) {
              if (!shouldInsertFunctionInstantiation(specializationId,
                                                     resolvedId))
                return;

              DbModel::FunctionInstantiation instantiation = {
                  specializationId, resolvedId};
              STG.insertClassObj(instantiation);
            }};
        DependencyManager::instance().addDependency(update);
      }
    }

    recordFunctionTemplateTypeArguments(
        specializationId, decl->getTemplateSpecializationArgs(),
        type_processor_.get(), context_);
    recordFunctionTemplateArgumentValues(
        specializationId, decl->getTemplateSpecializationArgsAsWritten(),
        expr_processor_.get(), context_);
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

  if (clang::TypeSourceInfo *typeInfo = decl->getTypeSourceInfo()) {
    auto templateTypeLoc = typeInfo->getTypeLoc()
                               .getAsAdjusted<clang::TemplateSpecializationTypeLoc>();
    recordClassTemplateArgumentValues(type_id, templateTypeLoc,
                                      expr_processor_.get(), context_);
  }

  if (const auto *specialization =
          llvm::dyn_cast_or_null<clang::VarTemplateSpecializationDecl>(decl)) {
    int variableId =
        resolveVariableEntityId(specialization, variable_processor_.get(),
                                context_);
    if (variableId == -1)
      return true;

    const clang::VarTemplateDecl *primaryTemplate =
        specialization->getSpecializedTemplate();
    if (primaryTemplate && primaryTemplate->getTemplatedDecl()) {
      int templateId = resolveVariableEntityId(
          primaryTemplate->getTemplatedDecl(), variable_processor_.get(),
          context_);
      if (templateId != -1 &&
          shouldInsertVariableInstantiation(variableId, templateId)) {
        DbModel::VariableInstantiation instantiation = {variableId,
                                                        templateId};
        STG.insertClassObj(instantiation);
      }
    }

    recordVariableTemplateTypeArguments(
        variableId, specialization->getTemplateInstantiationArgs(),
        type_processor_.get(), context_);
    recordVariableTemplateArgumentValues(
        variableId, specialization->getTemplateArgsAsWritten(),
        expr_processor_.get(), context_);
  }
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
  if (!decl)
    return true;

  LocIdPair *locIdPair = SrcLocRecorder::processDefault(decl, context_);
  const int friendDeclId = GENID(FriendDecl);
  int typeId = -1;
  int declId = -1;

  auto resolveRecordType = [&](const clang::RecordType *recordType) -> int {
    if (!recordType || !recordType->getDecl())
      return -1;

    type_processor_->processRecordType(recordType);
    KeyType typeKey = KeyGen::Type::makeKey(recordType->getDecl(), context_);
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
      KeyType functionKey = KeyGen::Function::makeKey(functionDecl, context_);
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
  return true;
}

bool ASTVisitor::VisitConceptDecl(clang::ConceptDecl *decl) {
  resolveConceptTemplateId(decl, context_);
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
  if (!decl)
    return true;

  const clang::ClassTemplateDecl *classTemplateDecl = nullptr;
  auto instantiatedFrom = decl->getInstantiatedFrom();
  if (!instantiatedFrom.isNull())
    classTemplateDecl = instantiatedFrom.dyn_cast<clang::ClassTemplateDecl *>();
  if (!classTemplateDecl)
    classTemplateDecl = decl->getSpecializedTemplate();
  if (!classTemplateDecl)
    return true;

  const clang::CXXRecordDecl *templatedDecl =
      classTemplateDecl->getTemplatedDecl();
  if (!templatedDecl)
    return true;

  KeyType specializationKey = KeyGen::Type::makeKey(decl, context_);
  if (!SEARCH_TYPE_CACHE(specializationKey))
    type_processor_->processRecordDeclType(decl);

  KeyType templateKey = KeyGen::Type::makeKey(templatedDecl, context_);
  if (!SEARCH_TYPE_CACHE(templateKey))
    type_processor_->processRecordDeclType(templatedDecl);

  int specializationId = -1;
  int templateId = -1;

  if (auto cachedId = SEARCH_TYPE_CACHE(specializationKey))
    specializationId = *cachedId;
  if (auto cachedId = SEARCH_TYPE_CACHE(templateKey))
    templateId = *cachedId;

  if (specializationId != -1 && templateId != -1) {
    if (shouldInsertClassInstantiation(specializationId, templateId)) {
      DbModel::ClassInstantiation instantiation = {specializationId,
                                                   templateId};
      STG.insertClassObj(instantiation);
    }
  }

  recordClassTemplateTypeArguments(
      specializationId, decl->getTemplateInstantiationArgs(),
      type_processor_.get(), context_);
  recordTemplateTemplateArguments(
      specializationId, decl->getTemplateInstantiationArgs(),
      type_processor_.get(), context_);
  recordClassTemplateArgumentValues(
      specializationId, decl->getTemplateArgsAsWritten(), expr_processor_.get(),
      context_);

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

  int variableId =
      resolveVariableEntityId(templatedDecl, variable_processor_.get(),
                              context_);
  if (variableId == -1 || !shouldInsertVariableTemplate(variableId))
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
      recordFunctionTemplateArgumentValues(
          functionId, expr->getTemplateArgs(), expr->getNumTemplateArgs(),
          expr_processor_.get(), context_);
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
  if (!expr)
    return true;

  int templateId = resolveConceptTemplateId(expr->getNamedConcept(), context_);
  if (templateId == -1)
    return true;

  int conceptId = resolveConceptSpecializationId(expr, context_);
  if (conceptId == -1)
    return true;

  if (shouldInsertConceptInstantiation(conceptId, templateId)) {
    DbModel::ConceptInstantiation instantiation = {conceptId, templateId};
    STG.insertClassObj(instantiation);
  }

  recordConceptTemplateTypeArguments(conceptId, expr->getTemplateArguments(),
                                     type_processor_.get(), context_);
  return true;
}
