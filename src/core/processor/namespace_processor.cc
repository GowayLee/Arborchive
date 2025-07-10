#include "core/processor/namespace_processor.h"
#include "db/storage_facade.h"
#include "model/db/container.h"
#include "util/id_generator.h"
#include "util/logger/macros.h"
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>

void NamespaceProcessor::processNamespaceDecl(
    const clang::NamespaceDecl *decl) {
  // Process the namespace itself
  processNamespace(decl);

  // Process inline namespace if applicable
  if (decl->isInline()) {
    processNamespaceInline(decl);
  }

  // Process namespace members
  processNamespaceMembers(decl);
}

void NamespaceProcessor::processNamespace(const clang::NamespaceDecl *decl) {
  int namespace_id = GENID(Namespace);
  std::string name = decl->getNameAsString();

  DbModel::Namespace namespace_record = {namespace_id, name};

  // Store in database using insertClassObj
  StorageFacade::getInstance().insertClassObj(namespace_record);

  LOG_DEBUG << "Processed namespace: " << name << " with ID: " << namespace_id
            << std::endl;
}

void NamespaceProcessor::processNamespaceInline(
    const clang::NamespaceDecl *decl) {
  int namespace_id = GENID(Namespace);

  DbModel::NamespaceInline namespace_inline_record = {namespace_id};

  // Store in database using insertClassObj
  StorageFacade::getInstance().insertClassObj(namespace_inline_record);

  LOG_DEBUG << "Processed inline namespace with ID: " << namespace_id
            << std::endl;
}

void NamespaceProcessor::processNamespaceMembers(
    const clang::NamespaceDecl *decl) {
  int parent_id = GENID(Namespace);

  // Iterate through all declarations in the namespace
  for (const auto *member : decl->decls()) {
    processNamespaceMember(decl, member);
  }
}

void NamespaceProcessor::processNamespaceMember(
    const clang::NamespaceDecl *parent_ns, const clang::Decl *member) {
  int parent_id = GENID(Namespace);
  // int member_id = GENID(NamespaceMember); // FIXME: About @ tables' issue

  DbModel::NamespaceMember namespace_member_record = {parent_id, -1};

  // Store in database using insertClassObj
  StorageFacade::getInstance().insertClassObj(namespace_member_record);

  LOG_DEBUG << "Processed namespace member with parent ID: " << parent_id
            << " and member ID: " << -1 << std::endl;
}