#ifndef _TYPE_PROCESSOR_H_
#define _TYPE_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/type.h"
#include <clang/AST/Decl.h>

using namespace clang;

class TypeProcessor : public BaseProcessor {
public:
  int processType(const Type *T);

  // Specific type declaration processing methods
  void processRecordDecl(const RecordDecl *RD);
  void processEnumDecl(const EnumDecl *ED);
  void processTypedefDecl(const TypedefDecl *TND);
  void processTemplateTypeParmDecl(const TemplateTypeParmDecl *TTPD);
  void processTypeDecl(const TypeDecl *TD);
  void processRecordType(const RecordType *RT);
  int processBuiltinType(const BuiltinType *BT, ASTContext *ast_context);

  TypeProcessor(ASTContext *ast_context, const PrintingPolicy pp)
      : BaseProcessor(ast_context, pp) {};
  ~TypeProcessor() = default;

  // Getter for the last processed type ID
  int getLastTypeId() const { return _typeId; }

private:
  int _typeId;
  int _typeDeclId;

  int processDerivedType(
      const Type *T,
      const std::optional<std::pair<DerivedTypeKind, QualType>> derived_result,
      ASTContext *ast_context);
  int processUserType(const Type *TP, ASTContext *ast_context);
  int processRoutineType(const Type *TP, ASTContext *ast_context);
  int processPtrToMemberType(const MemberPointerType *MPT,
                             ASTContext *ast_context);
  int processDeclType(const DecltypeType *DT, ASTContext *ast_context);
  // void processRecordType(const RecordType *RT);

  // New C language feature processing methods
  void processEnumConstants(const EnumDecl *ED, int parentEnumId);
  void processTypedefBase(const TypedefNameDecl *TND, int typedefId);
  void processArraySizes(const ArrayType *AT, int derivedTypeId);
  void processPointerishSize(const Type *T, int derivedTypeId);

  void recordTypeDef(const TypeDecl *TD);
  void recordTopTypeDecl(const TypeDecl *TD);
};

#endif // _TYPE_PROCESSOR_H_
