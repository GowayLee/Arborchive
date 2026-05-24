#ifndef _INHERITANCE_PROCESSOR_H_
#define _INHERITANCE_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include <clang/AST/DeclCXX.h>
#include <string>

class SpecifierProcessor;
class TypeProcessor;

class InheritanceProcessor : public BaseProcessor {
public:
  InheritanceProcessor(clang::ASTContext *ast_context,
                       const clang::PrintingPolicy pp,
                       TypeProcessor *type_processor,
                       SpecifierProcessor *specifier_processor)
      : BaseProcessor(ast_context, pp), type_processor_(type_processor),
        specifier_processor_(specifier_processor) {}

  void processCXXRecordDecl(const clang::CXXRecordDecl *decl);

  static std::string makeDerivationKey(int sub_id, int index, int super_id);

private:
  TypeProcessor *type_processor_ = nullptr;
  SpecifierProcessor *specifier_processor_ = nullptr;

  const clang::CXXRecordDecl *
  getProcessableDefinition(const clang::CXXRecordDecl *decl) const;
  int resolveRecordTypeId(const clang::CXXRecordDecl *decl);
  int resolveBaseTypeId(const clang::CXXBaseSpecifier &base,
                        bool &is_dependent,
                        std::string &dependent_super_name);
  void processBaseSpecifier(const clang::CXXBaseSpecifier &base, int index,
                            int sub_id);
  void recordDerivationSpecifiers(int derivation_id,
                                  const clang::CXXBaseSpecifier &base);
};

#endif // _INHERITANCE_PROCESSOR_H_
