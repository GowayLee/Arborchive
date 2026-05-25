#ifndef _RECORD_LAYOUT_PROCESSOR_H_
#define _RECORD_LAYOUT_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include <clang/AST/DeclCXX.h>
#include <cstdint>
#include <optional>
#include <string>

class TypeProcessor;
class VariableProcessor;

namespace clang {
class ASTRecordLayout;
class CharUnits;
} // namespace clang

class RecordLayoutProcessor : public BaseProcessor {
public:
  RecordLayoutProcessor(clang::ASTContext *ast_context,
                        const clang::PrintingPolicy pp,
                        TypeProcessor *type_processor,
                        VariableProcessor *variable_processor)
      : BaseProcessor(ast_context, pp), type_processor_(type_processor),
        variable_processor_(variable_processor) {}

  void processCXXRecordDecl(const clang::CXXRecordDecl *decl);

private:
  TypeProcessor *type_processor_ = nullptr;
  VariableProcessor *variable_processor_ = nullptr;

  const clang::CXXRecordDecl *
  getLayoutReadyDefinition(const clang::CXXRecordDecl *decl) const;
  int resolveRecordTypeId(const clang::CXXRecordDecl *decl);
  std::optional<int> findDerivationId(int sub_id, int index, int super_id);
  std::optional<int> toInt(clang::CharUnits units) const;
  std::optional<int> bitsToByteOffset(uint64_t bit_offset) const;
  std::optional<int> bitsToBitOffset(uint64_t bit_offset) const;
  int toIntFlag(bool value) const;

  void recordLayoutMetadata(const clang::CXXRecordDecl *decl,
                            const clang::ASTRecordLayout &layout, int sub_id);
  void recordDirectBaseOffsets(const clang::CXXRecordDecl *decl,
                               const clang::ASTRecordLayout &layout,
                               int sub_id);
  void recordVirtualBaseOffsets(const clang::CXXRecordDecl *decl,
                                const clang::ASTRecordLayout &layout,
                                int sub_id);
  void recordFieldOffsets(const clang::CXXRecordDecl *decl,
                          const clang::ASTRecordLayout &layout);
  void recordBitField(const clang::FieldDecl *field, int member_var_id);
  void recordFieldLayoutTraits(const clang::CXXRecordDecl *parent,
                               const clang::FieldDecl *field,
                               int member_var_id);
  void recordIndirectFieldPaths(const clang::CXXRecordDecl *decl, int sub_id);
  std::string buildIndirectFieldPath(const clang::IndirectFieldDecl *decl) const;
};

#endif // _RECORD_LAYOUT_PROCESSOR_H_
