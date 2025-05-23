#ifndef _VARIABLE_PROCESSOR_H_
#define _VARIABLE_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/function.h"
#include <clang/AST/Decl.h>

using namespace clang;

class VariableProcessor : public BaseProcessor {
public:
  void routerProcess(const VarDecl *VD);

  VariableProcessor(ASTContext *ast_context) : BaseProcessor(ast_context) {};
  ~VariableProcessor() = default;

private:
  int _typeId;
  int _varId;
  int _varDeclId;
  std::string _name;

  void recordSpecifier(const VarDecl *VD);
  void recordSpecialize(const VarDecl *VD);
  void recordStructuredBinding(const VarDecl *VD);
  void recordRequire(const VarDecl *VD);

  int processLocalScopeVar(const VarDecl *VD);
  int processGlobalVar(const VarDecl *VD);
  int processMemberVar(const VarDecl *VD);

  int processLocalVar(const VarDecl *VD);
  int processParam(const VarDecl *VD);
};

#endif // _VARIABLE_PROCESSOR_H_
