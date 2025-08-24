#include "core/processor/stmt_processor.h"
#include "core/srcloc_recorder.h"
#include "db/dependency_manager.h"
#include "db/storage_facade.h"
#include "model/db/stmt.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/stmt.h"
#include "util/key_generator/variable.h"
#include "util/logger/macros.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtCXX.h>
#include <clang/Basic/LLVM.h>

int StmtProcessor::getStmtId(Stmt *stmt, StmtKind stmtKind) {
  KeyType stmtKey = KeyGen::Stmt_::makeKey(stmt, *ast_context_);
  LocIdPair *locIdPair = SrcLocRecorder::processStmt(stmt, ast_context_);

  DbModel::Stmt stmtModel = {GENID(Stmt), static_cast<int>(stmtKind),
                             locIdPair->spec_id};

  INSERT_STMT_CACHE(stmtKey, stmtModel.id);
  STG.insertClassObj(stmtModel);
  return stmtModel.id;
}

void StmtProcessor::processIfStmt(IfStmt *ifStmt) {
  int if_stmt_id = getStmtId(ifStmt, StmtKind::IF);

  // 1. 处理初始化部分
  if (Stmt *init = ifStmt->getInit()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(init, *ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::IfInit ifInitModel = {if_stmt_id, *cachedId};
      STG.insertClassObj(ifInitModel);
    } else {
      DbModel::IfInit ifInitModel = {if_stmt_id, -1};
      STG.insertClassObj(ifInitModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [if_stmt_id](int resolvedId) {
            DbModel::IfInit updated_record = {if_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // 2. 处理then部分
  if (Stmt *then = ifStmt->getThen()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(then, *ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::IfThen ifThenModel = {if_stmt_id, *cachedId};
      STG.insertClassObj(ifThenModel);
    } else {
      DbModel::IfThen ifThenModel = {if_stmt_id, -1};
      STG.insertClassObj(ifThenModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [if_stmt_id](int resolvedId) {
            DbModel::IfThen updated_record = {if_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // 3. 处理else部分
  if (Stmt *elseStmt = ifStmt->getElse()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(elseStmt, *ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::IfElse ifElseModel = {if_stmt_id, *cachedId};
      STG.insertClassObj(ifElseModel);
    } else {
      DbModel::IfElse ifElseModel = {if_stmt_id, -1};
      STG.insertClassObj(ifElseModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [if_stmt_id](int resolvedId) {
            DbModel::IfElse updated_record = {if_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }
}

void StmtProcessor::processForStmt(ForStmt *forStmt) {
  int for_stmt_id = getStmtId(forStmt, StmtKind::FOR);

  // int for_or_range_id = GENID(StmtForOrRangeBased);
  // DbModel::StmtForOrRangeBased stmtForOrRangeBasedModel = {
  //     for_or_range_id, static_cast<int>(ForType::FOR), for_stmt_id};
  // STG.insertClassObj(stmtForOrRangeBasedModel);

  // 1. 处理初始化部分
  if (Stmt *init = forStmt->getInit()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(init, *ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::ForInit forInitModel = {for_stmt_id, *cachedId};
      STG.insertClassObj(forInitModel);
    } else {
      DbModel::ForInit forInitModel = {for_stmt_id, -1};
      STG.insertClassObj(forInitModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [for_stmt_id](int resolvedId) {
            DbModel::ForInit updated_record = {for_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // 2. 处理条件部分
  if (Expr *cond = forStmt->getCond()) {
    KeyType exprKey = KeyGen::Expr_::makeKey(cond, *ast_context_);
    if (auto cachedId = SEARCH_EXPR_CACHE(exprKey)) {
      DbModel::ForCond forCondModel = {for_stmt_id, *cachedId};
      STG.insertClassObj(forCondModel);
    } else {
      DbModel::ForCond forCondModel = {for_stmt_id, -1};
      STG.insertClassObj(forCondModel);
      PendingUpdate update{
          exprKey, CacheType::EXPR, [for_stmt_id](int resolvedId) {
            DbModel::ForCond updated_record = {for_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // 3. 处理更新部分
  if (Expr *inc = forStmt->getInc()) {
    KeyType exprKey = KeyGen::Expr_::makeKey(inc, *ast_context_);
    if (auto cachedId = SEARCH_EXPR_CACHE(exprKey)) {
      DbModel::ForUpdate forUpdateModel = {for_stmt_id, *cachedId};
      STG.insertClassObj(forUpdateModel);
    } else {
      DbModel::ForUpdate forUpdateModel = {for_stmt_id, -1};
      STG.insertClassObj(forUpdateModel);
      PendingUpdate update{
          exprKey, CacheType::EXPR, [for_stmt_id](int resolvedId) {
            DbModel::ForUpdate updated_record = {for_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // 4. 处理循环体
  if (Stmt *body = forStmt->getBody()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, *ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::ForBody forBodyModel = {for_stmt_id, *cachedId};
      STG.insertClassObj(forBodyModel);
    } else {
      DbModel::ForBody forBodyModel = {for_stmt_id, -1};
      STG.insertClassObj(forBodyModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [for_stmt_id](int resolvedId) {
            DbModel::ForBody updated_record = {for_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }
}

void StmtProcessor::processCXXForRangeStmt(CXXForRangeStmt *rangeForStmt) {
  int for_stmt_id = getStmtId((Stmt *)rangeForStmt, StmtKind::FOR);

  // int for_or_range_id = GENID(StmtForOrRangeBased);
  // DbModel::StmtForOrRangeBased stmtForOrRangeBasedModel = {
  //     for_or_range_id, static_cast<int>(ForType::RANGE_BASED_FOR),
  //     for_stmt_id};
  // STG.insertClassObj(stmtForOrRangeBasedModel);

  // 1. 处理范围声明（相当于初始化）
  if (Stmt *init = rangeForStmt->getInit()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(init, *ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::ForInit forInitModel = {for_stmt_id, *cachedId};
      STG.insertClassObj(forInitModel);
    } else {
      DbModel::ForInit forInitModel = {for_stmt_id, -1};
      STG.insertClassObj(forInitModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [for_stmt_id](int resolvedId) {
            DbModel::ForInit updated_record = {for_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }
}

void StmtProcessor::processWhileStmt(WhileStmt *whileStmt) {
  int while_stmt_id = getStmtId(whileStmt, StmtKind::WHILE);

  // 处理循环体
  if (Stmt *body = whileStmt->getBody()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, *ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::WhileBody whileBodyModel = {while_stmt_id, *cachedId};
      STG.insertClassObj(whileBodyModel);
    } else {
      DbModel::WhileBody whileBodyModel = {while_stmt_id, -1};
      STG.insertClassObj(whileBodyModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [while_stmt_id](int resolvedId) {
            DbModel::WhileBody updated_record = {while_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }
}

void StmtProcessor::processDoStmt(DoStmt *doStmt) {
  int do_stmt_id = getStmtId(doStmt, StmtKind::END_TEST_WHILE);

  // 处理循环体
  if (Stmt *body = doStmt->getBody()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, *ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::DoBody doBodyModel = {do_stmt_id, *cachedId};
      STG.insertClassObj(doBodyModel);
    } else {
      DbModel::DoBody doBodyModel = {do_stmt_id, -1};
      STG.insertClassObj(doBodyModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [do_stmt_id](int resolvedId) {
            DbModel::DoBody updated_record = {do_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }
}

void StmtProcessor::processSwitchStmt(SwitchStmt *switchStmt) {
  int switch_stmt_id = getStmtId(switchStmt, StmtKind::SWITCH);

  // 1. 处理初始化部分
  if (Stmt *init = switchStmt->getInit()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(init, *ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::SwitchInit switchInitModel = {switch_stmt_id, *cachedId};
      STG.insertClassObj(switchInitModel);
    } else {
      DbModel::SwitchInit switchInitModel = {switch_stmt_id, -1};
      STG.insertClassObj(switchInitModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [switch_stmt_id](int resolvedId) {
            DbModel::SwitchInit updated_record = {switch_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // 2. 处理主体部分
  if (Stmt *body = switchStmt->getBody()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, *ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::SwitchBody switchBodyModel = {switch_stmt_id, *cachedId};
      STG.insertClassObj(switchBodyModel);
    } else {
      DbModel::SwitchBody switchBodyModel = {switch_stmt_id, -1};
      STG.insertClassObj(switchBodyModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [switch_stmt_id](int resolvedId) {
            DbModel::SwitchBody updated_record = {switch_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }

    // 3. 处理 case 部分 - 遍历 body 中的 case 语句
    if (CompoundStmt *compoundBody = dyn_cast<CompoundStmt>(body)) {
      int case_index = 0;
      for (Stmt *child : compoundBody->children()) {
        if (SwitchCase *switchCase = dyn_cast<SwitchCase>(child)) {
          int case_id = getStmtId(switchCase, StmtKind::SWITCH_CASE);
          KeyType caseKey = KeyGen::Stmt_::makeKey(switchCase, *ast_context_);
          if (auto cachedId = SEARCH_STMT_CACHE(caseKey)) {
            DbModel::SwitchCase switchCaseModel = {switch_stmt_id, case_index,
                                                   *cachedId};
            STG.insertClassObj(switchCaseModel);
          } else {
            DbModel::SwitchCase switchCaseModel = {switch_stmt_id, case_index,
                                                   -1};
            STG.insertClassObj(switchCaseModel);
            PendingUpdate update{caseKey, CacheType::STMT,
                                 [switch_stmt_id, case_index](int resolvedId) {
                                   DbModel::SwitchCase updated_record = {
                                       switch_stmt_id, case_index, resolvedId};
                                   STG.insertClassObj(updated_record);
                                 }};
            DependencyManager::instance().addDependency(update);
          }
          case_index++;
        }
      }
    }
  }
}

void StmtProcessor::processBlockStmt(CompoundStmt *blockStmt) {
  int block_stmt_id = getStmtId(blockStmt, StmtKind::BLOCK);
}

void StmtProcessor::processReturnStmt(ReturnStmt *returnStmt) {
  // Just call getStmtId to create ReturnStmt record
  getStmtId(returnStmt, StmtKind::RETURN);
}

void StmtProcessor::processDeclStmt(DeclStmt *declStmt) {
  // Just call getStmtId to create DeclStmt record
  getStmtId(declStmt, StmtKind::DECL);
}
