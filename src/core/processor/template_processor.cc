#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include "core/processor/template_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/concept.h"
#include "util/id_generator.h"
#include "util/key_generator/type.h"
#include <clang/AST/ASTConcept.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/ExprConcepts.h>
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
