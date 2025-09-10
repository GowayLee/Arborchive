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

  TypeProcessor(ASTContext *ast_context) : BaseProcessor(ast_context) {};
  ~TypeProcessor() = default;

private:
  int _typeId;
  int _typeDeclId;

  int processBuiltinType(const BuiltinType *BT, ASTContext &ast_context);
  int processDerivedType(const Type *TP, ASTContext &ast_context);
  int processUserType(const Type *TP, ASTContext &ast_context);
  int processRoutineType(const Type *TP, ASTContext &ast_context);
  int processPtrToMemberType(const MemberPointerType *MPT, ASTContext &ast_context);
  int processDeclType(const DecltypeType *DT, ASTContext &ast_context);

  void recordTypeProcessing(const TypeDecl *TD);
  void recordTypeDef(const TypeDecl *TD);
  void recordTopTypeDecl(const TypeDecl *TD);
};

#endif // _TYPE_PROCESSOR_H_
