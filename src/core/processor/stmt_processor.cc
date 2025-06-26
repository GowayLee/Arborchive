#include "core/processor/stmt_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/stmt.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/stmt.h"
#include "util/logger/macros.h"
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtCXX.h>
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

void StmtProcessor::processForStmt(ForStmt *forStmt) {
  int for_stmt_id = getStmtId(forStmt, StmtType::FOR);

  // 0. Determine logacy for or range based for
  DbModel::StmtForOrRangeBased stmtForOrRangeBasedModel = {
      GENID(StmtForOrRangeBased), static_cast<int>(ForType::FOR), for_stmt_id};
  STG.insertClassObj(stmtForOrRangeBasedModel);

  // 1. 处理初始化部分
  if (Stmt *init = forStmt->getInit()) {
    int init_id = -1;
    KeyType stmtKey = KeyGen::Stmt_::makeKey(init, *ast_context_);
    // FIXME:
    DbModel::ForInit forInitModel = {stmtForOrRangeBasedModel.id, init_id};
    STG.insertClassObj(forInitModel);
  }

  // 2. 处理条件部分
  if (Expr *cond = forStmt->getCond()) {
    int condition_id = -1;
    KeyType exprKey = KeyGen::Expr_::makeKey(cond, *ast_context_);
    // FIXME:
    LOG_DEBUG << "For condition expr key: " << exprKey << std::endl;
    DbModel::ForCond forCondModel = {for_stmt_id, condition_id};
    STG.insertClassObj(forCondModel);
  }

  // 3. 处理更新部分
  if (Expr *inc = forStmt->getInc()) {
    int update_id = -1;
    KeyType exprKey = KeyGen::Expr_::makeKey(inc, *ast_context_);
    // FIXME:
    LOG_DEBUG << "For update expr key: " << exprKey << std::endl;
    DbModel::ForUpdate forUpdateModel = {for_stmt_id, update_id};
    STG.insertClassObj(forUpdateModel);
  }

  // 4. 处理循环体
  if (Stmt *body = forStmt->getBody()) {
    int body_id = -1;
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, *ast_context_);
    // FIXME:
    DbModel::ForBody forBodyModel = {for_stmt_id, body_id};
    STG.insertClassObj(forBodyModel);
  }
}

void StmtProcessor::processCXXForRangeStmt(CXXForRangeStmt *rangeForStmt) {
  int for_stmt_id = getStmtId((Stmt *)rangeForStmt, StmtType::FOR);

  // 0. Determine logacy for or range based for
  DbModel::StmtForOrRangeBased stmtForOrRangeBasedModel = {
      GENID(StmtForOrRangeBased), static_cast<int>(ForType::RANGE_BASED_FOR),
      for_stmt_id};
  STG.insertClassObj(stmtForOrRangeBasedModel);

  // 1. 处理范围声明（相当于初始化）
  if (Stmt *init = rangeForStmt->getInit()) {
    int init_id = -1;
    KeyType stmtKey = KeyGen::Stmt_::makeKey(init, *ast_context_);
    // FIXME:
    DbModel::ForInit forInitModel = {stmtForOrRangeBasedModel.id, init_id};
    STG.insertClassObj(forInitModel);
  }
}

void StmtProcessor::processWhileStmt(WhileStmt *whileStmt) {
  int while_stmt_id = getStmtId(whileStmt, StmtType::WHILE);

  // 处理循环体
  if (Stmt *body = whileStmt->getBody()) {
    int body_id = -1;
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, *ast_context_);
    // Since sub stmtNode of WHILE stmt will be visited after visiting root node of
    // WHILE, so, directly add this to dependency manager queue

    // TODO: Dependency Manager
    DbModel::WhileBody whileBodyModel = {while_stmt_id, body_id};
    STG.insertClassObj(whileBodyModel);
  }
}

void StmtProcessor::processDoStmt(DoStmt *doStmt) {
  int do_stmt_id = getStmtId(doStmt, StmtType::END_TEST_WHILE);

  // 处理循环体
  if (Stmt *body = doStmt->getBody()) {
    int body_id = -1;
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, *ast_context_);
    // Since sub stmtNode of DO stmt will be visited after visiting root node of
    // DO, so, directly add this to dependency manager queue

    // TODO: Dependency Manager
    DbModel::DoBody doBodyModel = {do_stmt_id, body_id};
    STG.insertClassObj(doBodyModel);
  }
}
