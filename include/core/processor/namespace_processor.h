#ifndef _NAMESPACE_PROCESSOR_H_
#define _NAMESPACE_PROCESSOR_H_

#include "base_processor.h"
#include <clang/AST/Decl.h>

class NamespaceProcessor : public BaseProcessor {
public:
  NamespaceProcessor(clang::ASTContext *ast_context)
      : BaseProcessor(ast_context) {}
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