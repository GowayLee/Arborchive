#include "core/processor/stmt_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/stmt.h"
#include "util/id_generator.h"
#include "util/key_generator/stmt.h"
#include "util/logger/macros.h"
#include <clang/AST/Stmt.h>
#include <clang/Basic/LLVM.h>

int StmtProcessor::getStmtId(Stmt *stmt, StmtType stmtType) {
  KeyType stmtKey = KeyGen::Stmt_::makeKey(stmt, *ast_context_);
  LocIdPair *locIdPair = SrcLocRecorder::processStmt(stmt, ast_context_);

  DbModel::Stmt stmtModel = {GENID(Stmt), static_cast<int>(stmtType),
                             locIdPair->spec_id};

  INSERT_STMT_CACHE(stmtKey, stmtModel.id);
  STG.insertClassObj(stmtModel);
  return stmtModel.id;
}

void StmtProcessor::processIfStmt(IfStmt *ifStmt) {
  int if_stmt_id = getStmtId(ifStmt, StmtType::IF);

  // 1. 处理初始化部分
  if (Stmt *init = ifStmt->getInit()) {
    int init_id = -1;
    KeyType stmtKey = KeyGen::Stmt_::makeKey(init, *ast_context_);
    // Since sub stmtNode of IF stmt will be visited after visiting root node of
    // IF, so, directly add this to dependency manager queue

    // TODO: Dependency Manager
    DbModel::IfInit ifInitModel = {if_stmt_id, init_id};
    STG.insertClassObj(ifInitModel);
  }

  // 2. 处理then部分
  if (Stmt *then = ifStmt->getThen()) {
    int then_id = -1;
    KeyType stmtKey = KeyGen::Stmt_::makeKey(then, *ast_context_);
    LOG_DEBUG << "if_then stmtKey: " << stmtKey << std::endl;
    // Since sub stmtNode of IF stmt will be visited after visiting root node of
    // IF, so, directly add this to dependency manager queue

    // TODO: Dependency Manager
    DbModel::IfThen ifInitModel = {if_stmt_id, then_id};
    STG.insertClassObj(ifInitModel);
  }

  // 3. 处理else部分
  if (Stmt *elseStmt = ifStmt->getElse()) {
    int else_id = -1;
    KeyType stmtKey = KeyGen::Stmt_::makeKey(elseStmt, *ast_context_);
    // Since sub stmtNode of IF stmt will be visited after visiting root node of
    // IF, so, directly add this to dependency manager queue

    // TODO: Dependency Manager
    DbModel::IfElse ifElseModel = {if_stmt_id, else_id};
    STG.insertClassObj(ifElseModel);
  }
}