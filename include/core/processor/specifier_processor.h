#ifndef _SPECIFIER_PROCESSOR_H_
#define _SPECIFIER_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "model/db/specifiers.h"
#include <clang/AST/Decl.h>

using namespace clang;

class SpecifierProcessor : public BaseProcessor {
public:
  // 获取或创建 specifier，返回其 id
  int getOrCreateSpecifier(const std::string &str);

  // 处理类型的 cvr 限定符 (const, volatile, restrict)
  void processTypeQualifiers(int type_id, QualType qualType);

  // 处理函数的说明符 (inline, static, virtual, constexpr 等)
  void processFunctionSpecifiers(int func_id, const FunctionDecl *FD);

  // 处理变量的说明符
  void processVariableSpecifiers(int var_id, const VarDecl *VD);

  SpecifierProcessor(ASTContext *ast_context, const PrintingPolicy pp)
      : BaseProcessor(ast_context, pp) {};
  ~SpecifierProcessor() = default;

private:
  // 插入 specifier 到关联表
  void insertTypeSpecifiers(int type_id, int spec_id);
  void insertFunSpecifiers(int func_id, int spec_id);
  void insertVarSpecifiers(int var_id, int spec_id);
};

#endif // _SPECIFIER_PROCESSOR_H_
