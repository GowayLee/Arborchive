#include "core/processor/inheritance_processor.h"
#include "core/processor/specifier_processor.h"
#include "core/processor/type_processor.h"
#include "core/srcloc_recorder.h"
#include "db/cache_repository.h"
#include "db/storage_facade.h"
#include "model/db/type.h"
#include "util/id_generator.h"
#include "util/logger/macros.h"
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Type.h>
#include <string>

std::string InheritanceProcessor::makeDerivationKey(int sub_id, int index,
                                                    int super_id) {
  return std::to_string(sub_id) + ":" + std::to_string(index) + ":" +
         std::to_string(super_id);
}

void InheritanceProcessor::processCXXRecordDecl(
    const clang::CXXRecordDecl *decl) {
  const clang::CXXRecordDecl *definition = getProcessableDefinition(decl);
  if (!definition || !type_processor_)
    return;

  const int subId = resolveRecordTypeId(definition);
  if (subId == -1)
    return;

  int index = 0;
  for (const clang::CXXBaseSpecifier &base : definition->bases()) {
    processBaseSpecifier(base, index, subId);
    ++index;
  }
}

const clang::CXXRecordDecl *
InheritanceProcessor::getProcessableDefinition(
    const clang::CXXRecordDecl *decl) const {
  if (!decl || decl->isInvalidDecl())
    return nullptr;

  const clang::CXXRecordDecl *definition = decl->getDefinition();
  if (!definition || definition != decl || definition->isInvalidDecl())
    return nullptr;

  return definition;
}

int InheritanceProcessor::resolveRecordTypeId(
    const clang::CXXRecordDecl *decl) {
  if (!decl || !type_processor_)
    return -1;

  const clang::CXXRecordDecl *definition = decl->getDefinition();
  if (definition)
    decl = definition;

  return type_processor_->processRecordDeclType(decl);
}

void InheritanceProcessor::processBaseSpecifier(
    const clang::CXXBaseSpecifier &base, int index, int sub_id) {
  if (sub_id == -1 || base.isPackExpansion())
    return;

  clang::QualType baseType = base.getType();
  if (baseType.isNull() || baseType->isDependentType())
    return;

  const clang::CXXRecordDecl *baseDecl = baseType->getAsCXXRecordDecl();
  if (!baseDecl)
    return;

  const int superId = resolveRecordTypeId(baseDecl);
  if (superId == -1)
    return;

  const std::string key = makeDerivationKey(sub_id, index, superId);
  auto &repo = CacheManager::instance()
                   .getRepository<CacheRepository<DbModel::Derivation>>();
  if (repo.find(key))
    return;

  int locationId = -1;
  if (base.getBeginLoc().isValid() && base.getEndLoc().isValid()) {
    LocIdPair *locIdPair =
        SrcLocRecorder::processDefault(base.getBeginLoc(), base.getEndLoc(),
                                       ast_context_);
    if (locIdPair)
      locationId = locIdPair->spec_id;
  }

  DbModel::Derivation derivation = {GENID(Derivation), sub_id, index, superId,
                                    locationId};
  repo.insert(key, derivation.id);
  STG.insertClassObj(derivation);

  recordDerivationSpecifiers(derivation.id, base);
}

void InheritanceProcessor::recordDerivationSpecifiers(
    int derivation_id, const clang::CXXBaseSpecifier &base) {
  if (derivation_id == -1 || !specifier_processor_)
    return;

  for (int specId : specifier_processor_->processBaseSpecifiers(base)) {
    DbModel::DerSpecifier derSpecifier = {derivation_id, specId};
    STG.insertClassObj(derSpecifier);
  }
}
