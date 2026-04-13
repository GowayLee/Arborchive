#ifndef _PREPROCESSOR_PROCESSOR_H_
#define _PREPROCESSOR_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/preprocessor.h"
#include "model/db/container.h"
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace clang;

class PreprocessorProcessor : public BaseProcessor, public PPCallbacks {
public:
  PreprocessorProcessor(ASTContext *ast_context, const PrintingPolicy pp,
                       class Preprocessor *preprocessor);
  ~PreprocessorProcessor() = default;

  ////// PPCallbacks Interface - Conditional Compilation //////

  void Ifndef(SourceLocation Loc, const Token &MacroNameTok,
              const MacroDefinition &MD) override;
  void Ifdef(SourceLocation Loc, const Token &MacroNameTok,
             const MacroDefinition &MD) override;
  void If(SourceLocation Loc, SourceRange ConditionRange,
          ConditionValueKind ConditionValue) override;
  void Elif(SourceLocation Loc, SourceRange ConditionRange,
            ConditionValueKind ConditionValue, SourceLocation IfLoc) override;
  void Else(SourceLocation Loc, SourceLocation IfLoc) override;
  void Endif(SourceLocation Loc, SourceLocation IfLoc) override;

  ////// PPCallbacks Interface - Include Directives //////

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                         StringRef FileName, bool IsAngled,
                         CharSourceRange FilenameRange,
                         OptionalFileEntryRef File,
                         StringRef SearchPath, StringRef RelativePath,
                         const Module *SuggestedModule, bool ModuleImported,
                         SrcMgr::CharacteristicKind FileType) override;

  ////// PPCallbacks Interface - Macro Definitions //////

  void MacroExpands(const Token &MacroNameTok, const MacroDefinition &MD,
                    SourceRange Range, const MacroArgs *Args) override;
  void MacroDefined(const Token &MacroNameTok, const MacroDirective *MD) override;
  void MacroUndefined(const Token &MacroNameTok, const MacroDefinition &MD,
                      const MacroDirective *Undef) override;
  void Defined(const Token &MacroNameTok, const MacroDefinition &MD,
               SourceRange Range) override;

  ////// PPCallbacks Interface - Other Directives //////

  void PragmaDirective(SourceLocation Loc, PragmaIntroducerKind Introducer) override;
  void PragmaMessage(SourceLocation Loc, StringRef Namespace, PragmaMessageKind Kind,
                     StringRef Str) override;
  void PragmaDebug(SourceLocation Loc, StringRef DebugType) override;

private:
  class Preprocessor *preprocessor_;

  // Branch tracking for if/elif/else/endif pairing
  struct BranchInfo {
    int directive_id;
    PreprocDirectKind kind;
    SourceLocation location;
  };
  std::stack<BranchInfo> branch_stack_;

  // Track which branches evaluated to true/false
  std::unordered_map<int, bool> branch_evaluation_;
  std::unordered_map<std::string, int> macro_define_id_by_name_;
  std::unordered_set<std::string> macro_argument_dedup_cache_;
  std::unordered_set<std::string> macrolocationbind_dedup_cache_;
  std::unordered_set<int> macroparent_child_dedup_cache_;
  std::unordered_map<unsigned, int> macro_invocation_by_loc_key_;

  static constexpr int kMacroExpansionKind = 1;
  static constexpr int kOtherMacroReferenceKind = 2;

  ////// Helper Methods //////

  // Process a preprocessor directive and return its ID
  int processDirective(SourceLocation Loc, PreprocDirectKind kind,
                       const std::string &key_suffix = "");

  // Extract and store directive text (head and body)
  void extractDirectiveText(SourceLocation Loc, int directive_id, PreprocDirectKind kind);

  // Record branch pairing (begin -> elif/else/endif)
  void recordBranchPair(int begin_id, int end_id);

  // Record whether branch evaluated to true or false
  void recordBranchEvaluation(int branch_id, bool is_true);

  // Resolve include file and create/update File entry
  int resolveIncludeFile(const std::string &filename);

  // Get full source text for a source range
  std::string getSourceText(CharSourceRange range);

  // Record macro invocation/reference row if matching define id is known.
  int recordMacroInvocation(const Token &MacroNameTok, SourceLocation Loc, int kind);

  // Convert tokens to a stable space-joined text representation.
  std::string tokensToText(const Token *tokens, unsigned token_count) const;
  std::string tokensToText(const std::vector<Token> &tokens) const;

  // Prevent duplicate insertions for the same (invocation, argument_index, table).
  bool shouldInsertMacroArgumentRow(int invocation_id, int argument_index,
                                    bool is_expanded);

  bool shouldInsertMacroLocationBindRow(int invocation_id, int location_id);
  bool shouldInsertMacroParentRow(int child_id);
  unsigned makeMacroLocationKey(SourceLocation loc) const;
};

#endif // _PREPROCESSOR_PROCESSOR_H_
