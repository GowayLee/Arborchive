#ifndef _TEMPLATE_PROCESSOR_H_
#define _TEMPLATE_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include <llvm/ADT/ArrayRef.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace clang {
class ConceptDecl;
class ConceptSpecializationExpr;
class Expr;
class QualType;
class ASTTemplateArgumentListInfo;
class ClassTemplateDecl;
class TemplateArgument;
class TemplateArgumentList;
class TemplateArgumentLoc;
class TemplateSpecializationTypeLoc;
class TemplateTemplateParmDecl;
class TemplateTypeParmDecl;
class VarDecl;
} // namespace clang

class ExprProcessor;
class TypeProcessor;
class VariableProcessor;

class TemplateProcessor : public BaseProcessor {
public:
  TemplateProcessor(clang::ASTContext *context, const clang::PrintingPolicy pp,
                    TypeProcessor *typeProcessor,
                    ExprProcessor *exprProcessor,
                    VariableProcessor *variableProcessor)
      : BaseProcessor(context, pp), type_processor_(typeProcessor),
        expr_processor_(exprProcessor),
        variable_processor_(variableProcessor) {
    (void)type_processor_;
    (void)expr_processor_;
    (void)variable_processor_;
  };
  ~TemplateProcessor() = default;

  bool shouldInsertClassInstantiation(int to, int from);
  bool shouldInsertClassTemplateArgument(int typeId, int index, int argType);
  bool shouldInsertClassTemplateArgumentValue(int typeId, int index,
                                              int argValue);
  bool shouldInsertFunctionInstantiation(int to, int from);
  bool shouldInsertFunctionTemplateArgument(int functionId, int index,
                                            int argType);
  bool shouldInsertFunctionTemplateArgumentValue(int functionId, int index,
                                                 int argValue);
  bool shouldInsertVariableTemplate(int variableId);
  bool shouldInsertVariableInstantiation(int to, int from);
  bool shouldInsertVariableTemplateArgument(int variableId, int index,
                                            int argType);
  bool shouldInsertVariableTemplateArgumentValue(int variableId, int index,
                                                 int argValue);
  bool shouldInsertTemplateTemplateInstantiation(int to, int from);
  bool shouldInsertTemplateTemplateArgument(int typeId, int index, int argType);
  bool shouldInsertConceptInstantiation(int conceptId, int templateId);
  bool shouldInsertConceptTemplateArgument(int conceptId, int index,
                                           int argType);
  bool shouldInsertTypeTemplateTypeConstraint(int typeId, int constraintId);
  bool shouldInsertIsTypeConstraint(int conceptId);
  bool shouldInsertNontypeTemplateParameter(int id);
  bool shouldInsertConceptTemplateArgumentValue(int conceptId, int index,
                                                int argValue);

  int resolveConceptTemplateId(const clang::ConceptDecl *decl,
                               clang::ASTContext *context);
  int resolveConceptSpecializationId(
      const clang::ConceptSpecializationExpr *expr,
      clang::ASTContext *context);
  // ---- 跨领域模板参数解析 ----
  int resolveVariableEntityId(const clang::VarDecl *decl);
  int resolveTemplateArgumentTypeId(clang::QualType argType);
  int resolveTemplateTemplateParmId(
      const clang::TemplateTemplateParmDecl *decl);
  int resolveTemplateTemplateArgumentTypeId(const clang::TemplateArgument &arg);
  int resolveTemplateArgumentExprId(const clang::Expr *sourceExpr);
  static const clang::Expr *getTemplateArgumentSourceExpr(
      const clang::TemplateArgumentLoc &argLoc);

  void recordClassTemplateTypeArguments(
      int typeId, const clang::TemplateArgumentList &templateArgs);
  void recordClassTemplateArgumentValues(
      int typeId, const clang::ASTTemplateArgumentListInfo *templateArgs);
  void recordClassTemplateArgumentValues(
      int typeId, clang::TemplateSpecializationTypeLoc templateArgs);
  void recordTemplateTemplateArguments(
      int typeId, const clang::TemplateArgumentList &templateArgs);
  void recordTemplateTemplateInstantiations(
      const clang::ClassTemplateDecl *classTemplateDecl,
      const clang::TemplateArgumentList &templateArgs);
  void recordFunctionTemplateTypeArguments(
      int functionId, const clang::TemplateArgumentList *templateArgs);
  void recordFunctionTemplateArgumentValues(
      int functionId, const clang::ASTTemplateArgumentListInfo *templateArgs);
  void recordFunctionTemplateArgumentValues(
      int functionId, const clang::TemplateArgumentLoc *templateArgs,
      unsigned numTemplateArgs);
  void recordVariableTemplateTypeArguments(
      int variableId, const clang::TemplateArgumentList &templateArgs);
  void recordVariableTemplateArgumentValues(
      int variableId, const clang::ASTTemplateArgumentListInfo *templateArgs);
  void recordConceptTemplateTypeArguments(
      int conceptId, llvm::ArrayRef<clang::TemplateArgument> templateArgs);
  void recordConceptTemplateArgumentValues(
      int conceptId, const clang::ConceptSpecializationExpr *expr);
  void recordTemplateTypeConstraint(const clang::TemplateTypeParmDecl *decl);

private:
  TypeProcessor *type_processor_ = nullptr;
  ExprProcessor *expr_processor_ = nullptr;
  VariableProcessor *variable_processor_ = nullptr;

  std::unordered_set<std::string> classInstantiationDedup;
  std::unordered_set<std::string> classTemplateArgumentDedup;
  std::unordered_set<std::string> classTemplateArgumentValueDedup;
  std::unordered_set<std::string> functionInstantiationDedup;
  std::unordered_set<std::string> functionTemplateArgumentDedup;
  std::unordered_set<std::string> functionTemplateArgumentValueDedup;
  std::unordered_set<std::string> variableTemplateDedup;
  std::unordered_set<std::string> variableInstantiationDedup;
  std::unordered_set<std::string> variableTemplateArgumentDedup;
  std::unordered_set<std::string> variableTemplateArgumentValueDedup;
  std::unordered_set<std::string> templateTemplateInstantiationDedup;
  std::unordered_set<std::string> templateTemplateArgumentDedup;
  std::unordered_set<std::string> conceptInstantiationDedup;
  std::unordered_set<std::string> conceptTemplateArgumentDedup;
  std::unordered_set<std::string> typeTemplateTypeConstraintDedup;
  std::unordered_set<int> isTypeConstraintDedup;
  std::unordered_set<int> nontypeTemplateParameterDedup;
  std::unordered_set<std::string> conceptTemplateArgumentValueDedup;
  std::unordered_map<std::string, int> conceptTemplateIds;
  std::unordered_map<std::string, int> conceptSpecializationIds;

  static std::string makePairKey(int first, int second);
  static std::string makeTripleKey(int first, int second, int third);
  static std::string makeConceptTemplateKey(const clang::ConceptDecl *decl,
                                            clang::ASTContext *context);
  static std::string makeTemplateArgumentListKey(
      llvm::ArrayRef<clang::TemplateArgument> args,
      clang::ASTContext *context);
  static std::string makeConceptSpecializationKey(
      const clang::ConceptSpecializationExpr *expr,
      clang::ASTContext *context);
};

#endif // _TEMPLATE_PROCESSOR_H_
