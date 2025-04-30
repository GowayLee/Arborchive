#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/location.h"
#include "util/id_generator.h"
#include "util/logger/macros.h"
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>
#include <filesystem>
#include <iostream>

using namespace DbModel;

LocIdPair *SrcLocRecorder::processDefault(const SourceLocation beginLoc,
                                          const SourceLocation endLoc,
                                          const ASTContext *context) {
  return process(beginLoc, endLoc, LocationType::DEFAULT, context);
}

LocIdPair *SrcLocRecorder::processStmt(const SourceLocation beginLoc,
                                       const SourceLocation endLoc,
                                       const ASTContext *context) {
  return process(beginLoc, endLoc, LocationType::STMT, context);
}

LocIdPair *SrcLocRecorder::processExpr(const SourceLocation beginLoc,
                                       const SourceLocation endLoc,
                                       const ASTContext *context) {
  return process(beginLoc, endLoc, LocationType::EXPR, context);
}

LocIdPair *SrcLocRecorder::processDefault(const Stmt *stmt,
                                          const ASTContext *context) {

  return process(stmt->getBeginLoc(), stmt->getEndLoc(), LocationType::DEFAULT,
                 context);
}

LocIdPair *SrcLocRecorder::processStmt(const Stmt *stmt,
                                       const ASTContext *context) {
  return process(stmt->getBeginLoc(), stmt->getEndLoc(), LocationType::STMT,
                 context);
}

LocIdPair *SrcLocRecorder::processExpr(const Stmt *stmt,
                                       const ASTContext *context) {
  return process(stmt->getBeginLoc(), stmt->getEndLoc(), LocationType::EXPR,
                 context);
}

LocIdPair *SrcLocRecorder::processDefault(const Decl *decl,
                                          const ASTContext *context) {

  return process(decl->getBeginLoc(), decl->getEndLoc(), LocationType::DEFAULT,
                 context);
}

LocIdPair *SrcLocRecorder::processStmt(const Decl *decl,
                                       const ASTContext *context) {
  return process(decl->getBeginLoc(), decl->getEndLoc(), LocationType::STMT,
                 context);
}

LocIdPair *SrcLocRecorder::processExpr(const Decl *decl,
                                       const ASTContext *context) {
  return process(decl->getBeginLoc(), decl->getEndLoc(), LocationType::EXPR,
                 context);
}

// 处理方法实现
LocIdPair *SrcLocRecorder::process(const SourceLocation beginLoc,
                                   const SourceLocation endLoc,
                                   const LocationType type,
                                   const ASTContext *context) {
  const auto &sourceManager = context->getSourceManager();

  if (beginLoc.isInvalid() || endLoc.isInvalid()) {
    LOG_WARNING << "Invalid source location for statement" << std::endl;
    return nullptr;
  }

  // 获取开始位置信息
  const unsigned start_line = sourceManager.getSpellingLineNumber(beginLoc);
  const unsigned start_column = sourceManager.getSpellingColumnNumber(beginLoc);

  // 获取结束位置信息
  const unsigned end_line = sourceManager.getSpellingLineNumber(endLoc);
  const unsigned end_column = sourceManager.getSpellingColumnNumber(endLoc);

  // 获取文件信息
  const FileID fileID = sourceManager.getFileID(beginLoc);
  const FileEntry *fileEntry = sourceManager.getFileEntryForID(fileID);
  std::string filename;

  if (fileEntry) {
    // 使用较新Clang版本中通用的方法获取文件名
    llvm::StringRef fileName = sourceManager.getFilename(beginLoc);
    filename = std::filesystem::path(fileName.str()).filename().string();
  } else {
    LOG_WARNING << "Could not determine file for statement" << std::endl;
    return nullptr;
  }

  // 创建位置模型
  // FIXME: Currently, only one source file will be parsed, so container_id here
  // is always 0
  Location locModel;
  LocIdPair *result = nullptr;

  switch (type) {
  case LocationType::DEFAULT: {
    LocationDefault locDefaultModel = {
        GENID(LocationDefault),       0,
        static_cast<int>(start_line), static_cast<int>(start_column),
        static_cast<int>(end_line),   static_cast<int>(end_column)};
    locModel = {GENID(Location), locDefaultModel.id};

    STG.insertClassObj(locDefaultModel);
    STG.insertClassObj(locModel);
    result = new LocIdPair{locModel.id, locDefaultModel.id};
    break;
  }
  case LocationType::STMT: {
    LocationStmt locStmtModel = {
        GENID(LocationStmt),          0,
        static_cast<int>(start_line), static_cast<int>(start_column),
        static_cast<int>(end_line),   static_cast<int>(end_column)};
    locModel = {GENID(Location), locStmtModel.id};

    STG.insertClassObj(locStmtModel);
    STG.insertClassObj(locModel);
    result = new LocIdPair{locModel.id, locStmtModel.id};
    break;
  }
  case LocationType::EXPR: {
    LocationExpr locExprModel = {
        GENID(LocationExpr),          0,
        static_cast<int>(start_line), static_cast<int>(start_column),
        static_cast<int>(end_line),   static_cast<int>(end_column)};
    locModel = {GENID(Location), locExprModel.id};

    STG.insertClassObj(locExprModel);
    STG.insertClassObj(locModel);
    result = new LocIdPair{locModel.id, locExprModel.id};
    break;
  }
  }

  LOG_DEBUG << "Recorded statement location: " << start_line << ":"
            << start_column << "-" << end_line << ":" << end_column
            << std::endl;

  return result;
}
