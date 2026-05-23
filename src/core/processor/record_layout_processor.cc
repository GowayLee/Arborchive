#include "core/processor/record_layout_processor.h"
#include "core/processor/inheritance_processor.h"
#include "core/processor/type_processor.h"
#include "core/processor/variable_processor.h"
#include "db/cache_repository.h"
#include "db/storage_facade.h"
#include "model/db/type.h"
#include "model/db/variable.h"
#include "util/logger/macros.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/CharUnits.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/RecordLayout.h>
#include <clang/AST/Type.h>
#include <cstdint>
#include <exception>
#include <limits>
#include <optional>
#include <string>

void RecordLayoutProcessor::processCXXRecordDecl(
    const clang::CXXRecordDecl *decl) {
  const clang::CXXRecordDecl *definition = getLayoutReadyDefinition(decl);
  if (!definition || !type_processor_)
    return;

  const int subId = resolveRecordTypeId(definition);
  if (subId == -1)
    return;

  try {
    const clang::ASTRecordLayout &layout =
        ast_context_->getASTRecordLayout(definition);
    recordDirectBaseOffsets(definition, layout, subId);
    recordVirtualBaseOffsets(definition, layout, subId);
    recordFieldOffsets(definition, layout);
  } catch (const std::exception &ex) {
    LOG_WARNING << "Skipping record layout for " << definition->getNameAsString()
                << ": " << ex.what() << std::endl;
  } catch (...) {
    LOG_WARNING << "Skipping record layout for " << definition->getNameAsString()
                << ": unknown layout failure" << std::endl;
  }
}

const clang::CXXRecordDecl *RecordLayoutProcessor::getLayoutReadyDefinition(
    const clang::CXXRecordDecl *decl) const {
  if (!decl || !ast_context_ || decl->isInvalidDecl())
    return nullptr;

  const clang::CXXRecordDecl *definition = decl->getDefinition();
  if (!definition || definition != decl || definition->isInvalidDecl())
    return nullptr;

  const clang::Type *recordType = definition->getTypeForDecl();
  if (!recordType || recordType->isDependentType())
    return nullptr;

  if (definition->isDependentContext() || definition->hasAnyDependentBases())
    return nullptr;

  return definition;
}

int RecordLayoutProcessor::resolveRecordTypeId(
    const clang::CXXRecordDecl *decl) {
  if (!decl || !type_processor_)
    return -1;

  const clang::CXXRecordDecl *definition = decl->getDefinition();
  if (definition)
    decl = definition;

  return type_processor_->processRecordDeclType(decl);
}

std::optional<int> RecordLayoutProcessor::findDerivationId(int sub_id,
                                                           int index,
                                                           int super_id) {
  const std::string key =
      InheritanceProcessor::makeDerivationKey(sub_id, index, super_id);
  return CacheManager::instance()
      .getRepository<CacheRepository<DbModel::Derivation>>()
      .find(key);
}

std::optional<int> RecordLayoutProcessor::toInt(clang::CharUnits units) const {
  const auto quantity = units.getQuantity();
  if (quantity < std::numeric_limits<int>::min() ||
      quantity > std::numeric_limits<int>::max())
    return std::nullopt;
  return static_cast<int>(quantity);
}

std::optional<int>
RecordLayoutProcessor::bitsToByteOffset(uint64_t bit_offset) const {
  if (!ast_context_)
    return std::nullopt;

  const uint64_t charWidth = ast_context_->getCharWidth();
  if (charWidth == 0)
    return std::nullopt;

  const uint64_t byteOffset = bit_offset / charWidth;
  if (byteOffset > static_cast<uint64_t>(std::numeric_limits<int>::max()))
    return std::nullopt;
  return static_cast<int>(byteOffset);
}

std::optional<int>
RecordLayoutProcessor::bitsToBitOffset(uint64_t bit_offset) const {
  if (!ast_context_)
    return std::nullopt;

  const uint64_t charWidth = ast_context_->getCharWidth();
  if (charWidth == 0)
    return std::nullopt;

  const uint64_t intraByteOffset = bit_offset % charWidth;
  if (intraByteOffset > static_cast<uint64_t>(std::numeric_limits<int>::max()))
    return std::nullopt;
  return static_cast<int>(intraByteOffset);
}

void RecordLayoutProcessor::recordDirectBaseOffsets(
    const clang::CXXRecordDecl *decl, const clang::ASTRecordLayout &layout,
    int sub_id) {
  if (!decl || sub_id == -1)
    return;

  int index = 0;
  for (const clang::CXXBaseSpecifier &base : decl->bases()) {
    if (base.isVirtual() || base.isPackExpansion()) {
      ++index;
      continue;
    }

    clang::QualType baseType = base.getType();
    if (baseType.isNull() || baseType->isDependentType()) {
      ++index;
      continue;
    }

    const clang::CXXRecordDecl *baseDecl = baseType->getAsCXXRecordDecl();
    if (!baseDecl) {
      ++index;
      continue;
    }

    const clang::CXXRecordDecl *baseDefinition = baseDecl->getDefinition();
    if (!baseDefinition) {
      ++index;
      continue;
    }

    const int superId = resolveRecordTypeId(baseDefinition);
    const std::optional<int> derivationId =
        findDerivationId(sub_id, index, superId);
    if (!derivationId) {
      LOG_WARNING << "Skipping direct base offset because derivation was not "
                     "found for sub="
                  << sub_id << ", index=" << index << ", super=" << superId
                  << std::endl;
      ++index;
      continue;
    }

    const std::optional<int> offset =
        toInt(layout.getBaseClassOffset(baseDefinition));
    if (offset) {
      DbModel::DirectBaseOffset directBaseOffset = {*derivationId, *offset};
      STG.insertClassObj(directBaseOffset);
    }
    ++index;
  }
}

void RecordLayoutProcessor::recordVirtualBaseOffsets(
    const clang::CXXRecordDecl *decl, const clang::ASTRecordLayout &layout,
    int sub_id) {
  if (!decl || sub_id == -1)
    return;

  for (const clang::CXXBaseSpecifier &base : decl->vbases()) {
    if (base.isPackExpansion())
      continue;

    clang::QualType baseType = base.getType();
    if (baseType.isNull() || baseType->isDependentType())
      continue;

    const clang::CXXRecordDecl *baseDecl = baseType->getAsCXXRecordDecl();
    if (!baseDecl)
      continue;

    const clang::CXXRecordDecl *baseDefinition = baseDecl->getDefinition();
    if (!baseDefinition)
      continue;

    const int superId = resolveRecordTypeId(baseDefinition);
    if (superId == -1)
      continue;

    const std::optional<int> offset =
        toInt(layout.getVBaseClassOffset(baseDefinition));
    if (offset) {
      DbModel::VirtualBaseOffset virtualBaseOffset = {sub_id, superId,
                                                      *offset};
      STG.insertClassObj(virtualBaseOffset);
    }
  }
}

void RecordLayoutProcessor::recordFieldOffsets(
    const clang::CXXRecordDecl *decl, const clang::ASTRecordLayout &layout) {
  if (!decl || !type_processor_ || !variable_processor_)
    return;

  for (const clang::FieldDecl *field : decl->fields()) {
    if (!field || field->isInvalidDecl() || field->isUnnamedBitField())
      continue;

    const unsigned fieldIndex = field->getFieldIndex();
    if (fieldIndex >= layout.getFieldCount())
      continue;

    const int typeId =
        type_processor_->processType(field->getType().getTypePtr());
    const int memberVarId =
        variable_processor_->resolveMemberVarId(field, typeId);
    if (memberVarId == -1)
      continue;

    const uint64_t bitOffset = layout.getFieldOffset(fieldIndex);
    const std::optional<int> byteOffset = bitsToByteOffset(bitOffset);
    const std::optional<int> intraByteOffset = bitsToBitOffset(bitOffset);
    if (!byteOffset || !intraByteOffset)
      continue;

    DbModel::FieldOffset fieldOffset = {memberVarId, *byteOffset,
                                        *intraByteOffset};
    STG.insertClassObj(fieldOffset);
  }
}
