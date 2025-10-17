#ifndef _SRCLOC_RECORDER_H_
#define _SRCLOC_RECORDER_H_

#include "model/db/location.h"
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceLocation.h>

using namespace clang;

// Struct as the return type for location and specification IDs
struct LocIdPair {
  int location_id;
  int spec_id;
};

class SrcLocRecorder {
public:
  static LocIdPair *processDefault(const Stmt *stmt, const ASTContext &context);
  static LocIdPair *processStmt(const Stmt *stmt, const ASTContext &context);
  static LocIdPair *processExpr(const Stmt *stmt, const ASTContext &context);
  static LocIdPair *processDefault(const Decl *decl, const ASTContext &context);
  static LocIdPair *processStmt(const Decl *decl, const ASTContext &context);
  static LocIdPair *processExpr(const Decl *decl, const ASTContext &context);
  static LocIdPair *processDefault(const SourceLocation beginLoc,
                                   const SourceLocation endLoc,
                                   const ASTContext &context);
  static LocIdPair *processStmt(const SourceLocation beginLoc,
                                const SourceLocation endLoc,
                                const ASTContext &context);
  static LocIdPair *processExpr(const SourceLocation beginLoc,
                                const SourceLocation endLoc,
                                const ASTContext &context);

private:
  static LocIdPair *process(const SourceLocation beginLoc,
                            const SourceLocation endLoc,
                            const LocationType type, const ASTContext &context);
};

#define PROC_DEFT SrcLocRecorder::processDefault
#define PROC_STMT SrcLocRecorder::processStmt
#define PROC_EXPR SrcLocRecorder::processExpr

#endif // _SRCLOC_RECORDER_H_
