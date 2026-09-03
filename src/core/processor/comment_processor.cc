#include "core/processor/comment_processor.h"
#include "core/compilation_recorder.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/comment.h"
#include "util/id_generator.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RawCommentList.h>
#include <clang/Basic/SourceManager.h>
#include <cstdint>
#include <memory>

std::string CommentProcessor::makeDedupKey(
    const clang::FunctionDecl *canonical_decl,
    const clang::RawComment &comment) const {
  const clang::SourceRange range = comment.getSourceRange();
  return std::to_string(reinterpret_cast<std::uintptr_t>(canonical_decl)) + ":" +
         std::to_string(range.getBegin().getRawEncoding()) + ":" +
         std::to_string(range.getEnd().getRawEncoding());
}

void CommentProcessor::processFunctionComment(
    const clang::FunctionDecl *decl, int function_entity_id) {
  if (!decl || decl->isImplicit() || function_entity_id < 0 || !ast_context_)
    return;

  const clang::FunctionDecl *canonical_decl = decl->getCanonicalDecl();
  const int canonical_function_id =
      canonical_function_ids_.emplace(canonical_decl, function_entity_id)
          .first->second;

  const clang::Decl *commented_decl = nullptr;
  const clang::RawComment *comment =
      ast_context_->getRawCommentForAnyRedecl(decl, &commented_decl);
  if (!comment || !comment->isDocumentation() || !commented_decl)
    return;

  const clang::SourceManager &source_manager = ast_context_->getSourceManager();
  const clang::SourceRange range = comment->getSourceRange();
  if (range.getBegin().isInvalid() || range.getEnd().isInvalid() ||
      range.getBegin().isMacroID() || range.getEnd().isMacroID() ||
      commented_decl->getLocation().isMacroID() ||
      !source_manager.isWrittenInMainFile(range.getBegin()) ||
      !source_manager.isWrittenInMainFile(range.getEnd()))
    return;

  const std::string dedup_key = makeDedupKey(canonical_decl, *comment);
  if (!processed_comments_.insert(dedup_key).second)
    return;

  const std::optional<int> source_file_id =
      CompRecorder::getInstance().getSourceFileId();
  if (!source_file_id || *source_file_id < 0)
    return;

  std::unique_ptr<LocIdPair> location(SrcLocRecorder::processDefault(
      range.getBegin(), range.getEnd(), ast_context_, *source_file_id));
  if (!location)
    return;

  const std::string formatted_text =
      comment->getFormattedText(source_manager, ast_context_->getDiagnostics());
  DbModel::Comment comment_model = {
      GENID(Comment),
      llvm::StringRef(formatted_text).trim().str(),
      location->spec_id};
  DbModel::CommentBinding binding_model = {comment_model.id,
                                           canonical_function_id};
  STG.insertClassObj(comment_model);
  STG.insertClassObj(binding_model);
}
