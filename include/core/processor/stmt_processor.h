#ifndef _STMT_PROCESSOR_H_
#define _STMT_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/stmt.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class StmtProcessor : public BaseProcessor {
public:
  int getStmtId(Stmt *stmt, StmtKind stmtKind);

  void processIfStmt(IfStmt *ifStmt);
  void processForStmt(ForStmt *forStmt);
  void processCXXForRangeStmt(CXXForRangeStmt *rangeForStmt);
  void processWhileStmt(WhileStmt *whileStmt);
  void processDoStmt(DoStmt *doStmt);
  void processSwitchStmt(SwitchStmt *switchStmt);
  void processBlockStmt(CompoundStmt *blockStmt);
  void processReturnStmt(ReturnStmt *returnStmt);
  void processDeclStmt(DeclStmt *declStmt);

  StmtProcessor(ASTContext *ast_context, const PrintingPolicy pp)
      : BaseProcessor(ast_context, pp) {};
  ~StmtProcessor() = default;

private:
  int _typeId;
  int _varId;
  int _varDeclId;
  std::string _name;
};

#endif // _STMT_PROCESSOR_H_
