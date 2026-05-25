#ifndef _NAMESPACE_PROCESSOR_H_
#define _NAMESPACE_PROCESSOR_H_

#include "base_processor.h"
#include <clang/AST/Decl.h>
#include <optional>
#include <unordered_map>

class NamespaceProcessor : public BaseProcessor {
public:
  NamespaceProcessor(clang::ASTContext *ast_context,
                     const clang::PrintingPolicy pp)
      : BaseProcessor(ast_context, pp) {}
  ~NamespaceProcessor() = default;

  void processNamespaceDecl(const clang::NamespaceDecl *decl);
  void processUsingDecl(const clang::UsingDecl *decl);
  void processUsingDirectiveDecl(const clang::UsingDirectiveDecl *decl);
  void processUnresolvedUsingTypenameDecl(
      const clang::UnresolvedUsingTypenameDecl *decl);
  void processNamespaceMember(const clang::NamespaceDecl *parent_ns,
                              const clang::Decl *member);

private:
  int getOrCreateNamespaceId(const clang::NamespaceDecl *decl);

  void processNamespace(const clang::NamespaceDecl *decl);
  void recordNamespaceDecl(const clang::NamespaceDecl *decl);
  void processNamespaceInline(const clang::NamespaceDecl *decl);
  void processNamespaceMembers(const clang::NamespaceDecl *decl);
  void recordUsing(const clang::Decl *decl, int kind);
  std::optional<int> resolveUsingOwnerElement(
      const clang::DeclContext *context);

  std::unordered_map<const clang::NamespaceDecl *, int> namespace_ids_;
};

#endif // _NAMESPACE_PROCESSOR_H_
