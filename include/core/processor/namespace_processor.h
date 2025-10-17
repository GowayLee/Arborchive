#ifndef _NAMESPACE_PROCESSOR_H_
#define _NAMESPACE_PROCESSOR_H_

#include "base_processor.h"
#include <clang/AST/Decl.h>

class NamespaceProcessor : public BaseProcessor {
public:
  NamespaceProcessor(const clang::ASTContext &ast_context,
                     const clang::PrintingPolicy pp)
      : BaseProcessor(ast_context, pp) {}
  ~NamespaceProcessor() = default;

  void processNamespaceDecl(const clang::NamespaceDecl *decl);
  void processNamespaceMember(const clang::NamespaceDecl *parent_ns,
                              const clang::Decl *member);

private:
  void processNamespace(const clang::NamespaceDecl *decl);
  void processNamespaceInline(const clang::NamespaceDecl *decl);
  void processNamespaceMembers(const clang::NamespaceDecl *decl);
};

#endif // _NAMESPACE_PROCESSOR_H_
