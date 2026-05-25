#include "core/processor/namespace_processor.h"
#include "core/compilation_recorder.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/container.h"
#include "model/db/declaration.h"
#include "util/id_generator.h"
#include "util/logger/macros.h"
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <llvm/Support/Casting.h>

void NamespaceProcessor::processNamespaceDecl(
    const clang::NamespaceDecl *decl) {
  // Process the namespace itself
  processNamespace(decl);

  // Process this concrete namespace declaration.
  recordNamespaceDecl(decl);

  // Process inline namespace if applicable
  if (decl->isInline()) {
    processNamespaceInline(decl);
  }

  // Process namespace members
  processNamespaceMembers(decl);
}

void NamespaceProcessor::processUsingDecl(const clang::UsingDecl *decl) {
  recordUsing(decl, 1);
}

void NamespaceProcessor::processUsingDirectiveDecl(
    const clang::UsingDirectiveDecl *decl) {
  recordUsing(decl, 2);
}

void NamespaceProcessor::processUnresolvedUsingTypenameDecl(
    const clang::UnresolvedUsingTypenameDecl *decl) {
  recordUsing(decl, 1);
}

int NamespaceProcessor::getOrCreateNamespaceId(
    const clang::NamespaceDecl *decl) {
  if (!decl) {
    return -1;
  }

  const clang::NamespaceDecl *canonical_decl = decl->getCanonicalDecl();
  auto existing = namespace_ids_.find(canonical_decl);
  if (existing != namespace_ids_.end()) {
    return existing->second;
  }

  int namespace_id = GENID(Namespace);
  std::string name = canonical_decl->getNameAsString();

  DbModel::Namespace namespace_record = {namespace_id, name};
  StorageFacade::getInstance().insertClassObj(namespace_record);
  namespace_ids_.emplace(canonical_decl, namespace_id);

  LOG_DEBUG << "Processed namespace: " << name << " with ID: "
            << namespace_id << std::endl;
  return namespace_id;
}

void NamespaceProcessor::processNamespace(const clang::NamespaceDecl *decl) {
  getOrCreateNamespaceId(decl);
}

void NamespaceProcessor::recordNamespaceDecl(
    const clang::NamespaceDecl *decl) {
  int namespace_id = getOrCreateNamespaceId(decl);
  if (namespace_id < 0) {
    return;
  }

  LocIdPair *decl_loc = PROC_DEFT(decl, ast_context_);
  clang::SourceLocation rbrace_loc = decl->getRBraceLoc();
  if (rbrace_loc.isInvalid()) {
    rbrace_loc = decl->getEndLoc();
  }
  LocIdPair *body_loc = PROC_DEFT(rbrace_loc, rbrace_loc, ast_context_);

  DbModel::NamespaceDecl namespace_decl_record = {
      GENID(NamespaceDecl), namespace_id, decl_loc ? decl_loc->spec_id : -1,
      body_loc ? body_loc->spec_id : -1};

  StorageFacade::getInstance().insertClassObj(namespace_decl_record);

  LOG_DEBUG << "Processed namespace declaration with ID: "
            << namespace_decl_record.id << " for namespace ID: "
            << namespace_id << std::endl;
}

void NamespaceProcessor::processNamespaceInline(
    const clang::NamespaceDecl *decl) {
  int namespace_id = getOrCreateNamespaceId(decl);
  if (namespace_id < 0) {
    return;
  }

  DbModel::NamespaceInline namespace_inline_record = {namespace_id};

  // Store in database using insertClassObj
  StorageFacade::getInstance().insertClassObj(namespace_inline_record);

  LOG_DEBUG << "Processed inline namespace with ID: " << namespace_id
            << std::endl;
}

void NamespaceProcessor::processNamespaceMembers(
    const clang::NamespaceDecl *decl) {
  // Iterate through all declarations in the namespace
  for (const auto *member : decl->decls()) {
    processNamespaceMember(decl, member);
  }
}

void NamespaceProcessor::recordUsing(const clang::Decl *decl, int kind) {
  if (!decl) {
    return;
  }
  if (decl->isImplicit()) {
    LOG_DEBUG << "Skipping implicit using declaration" << std::endl;
    return;
  }

  LocIdPair *decl_loc = PROC_DEFT(decl, ast_context_);
  DbModel::Using using_record = {GENID(Using), -1,
                                 decl_loc ? decl_loc->spec_id : -1, kind};

  StorageFacade::getInstance().insertClassObj(using_record);

  if (auto parent_id = resolveUsingOwnerElement(decl->getDeclContext())) {
    DbModel::UsingContainer using_container = {*parent_id, using_record.id};
    StorageFacade::getInstance().insertClassObj(using_container);
  } else {
    LOG_DEBUG << "Skipped using_container for unsupported owner context"
              << std::endl;
  }

  LOG_DEBUG << "Processed using with ID: " << using_record.id
            << " and kind: " << kind << std::endl;
}

std::optional<int> NamespaceProcessor::resolveUsingOwnerElement(
    const clang::DeclContext *context) {
  if (!context) {
    return std::nullopt;
  }

  if (const auto *namespace_decl =
          llvm::dyn_cast<clang::NamespaceDecl>(context)) {
    int namespace_id = getOrCreateNamespaceId(namespace_decl);
    if (namespace_id >= 0) {
      return namespace_id;
    }
    return std::nullopt;
  }

  if (llvm::isa<clang::TranslationUnitDecl>(context)) {
    return CompRecorder::getInstance().getSourceFileId();
  }

  return std::nullopt;
}

void NamespaceProcessor::processNamespaceMember(
    const clang::NamespaceDecl *parent_ns, const clang::Decl *member) {
  int parent_id = getOrCreateNamespaceId(parent_ns);
  if (parent_id < 0) {
    return;
  }

  int member_id = -1;
  if (const auto *member_ns =
          llvm::dyn_cast_or_null<clang::NamespaceDecl>(member)) {
    member_id = getOrCreateNamespaceId(member_ns);
  }

  DbModel::NamespaceMember namespace_member_record = {parent_id, member_id};

  // Store in database using insertClassObj
  StorageFacade::getInstance().insertClassObj(namespace_member_record);

  LOG_DEBUG << "Processed namespace member with parent ID: " << parent_id
            << " and member ID: " << member_id << std::endl;
}
