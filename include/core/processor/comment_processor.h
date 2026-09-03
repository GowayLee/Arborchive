#ifndef _COMMENT_PROCESSOR_H_
#define _COMMENT_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace clang {
class FunctionDecl;
class RawComment;
} // namespace clang

class CommentProcessor : public BaseProcessor {
public:
  CommentProcessor(clang::ASTContext *ast_context,
                   const clang::PrintingPolicy pp)
      : BaseProcessor(ast_context, pp) {}

  void processFunctionComment(const clang::FunctionDecl *decl,
                              int function_entity_id);

private:
  std::string makeDedupKey(const clang::FunctionDecl *canonical_decl,
                           const clang::RawComment &comment) const;

  std::unordered_map<const clang::FunctionDecl *, int> canonical_function_ids_;
  std::unordered_set<std::string> processed_comments_;
};

#endif // _COMMENT_PROCESSOR_H_
