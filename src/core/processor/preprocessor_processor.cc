#include "core/processor/preprocessor_processor.h"
#include "db/storage_facade.h"
#include "model/db/location.h"
#include "util/id_generator.h"
#include "util/key_generator/preprocessor.h"
#include "util/logger/macros.h"
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <filesystem>

using namespace DbModel;

PreprocessorProcessor::PreprocessorProcessor(ASTContext *ast_context,
                                             const PrintingPolicy pp,
                                             class Preprocessor *preprocessor)
    : BaseProcessor(ast_context, pp), preprocessor_(preprocessor) {
  LOG_INFO << "PreprocessorProcessor initialized" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////
// Conditional Compilation Callbacks
//////////////////////////////////////////////////////////////////////////////

void PreprocessorProcessor::Ifndef(SourceLocation Loc, const Token &MacroNameTok,
                                   const MacroDefinition &MD) {
  int dir_id = processDirective(Loc, PreprocDirectKind::IFNDEF);
  branch_stack_.push({dir_id, PreprocDirectKind::IFNDEF, Loc});

  // #ifndef macro is NOT defined -> branch is true
  bool evaluated = !MD;
  recordBranchEvaluation(dir_id, evaluated);

  extractDirectiveText(Loc, dir_id, PreprocDirectKind::IFNDEF);
}

void PreprocessorProcessor::Ifdef(SourceLocation Loc, const Token &MacroNameTok,
                                  const MacroDefinition &MD) {
  int dir_id = processDirective(Loc, PreprocDirectKind::IFDEF);
  branch_stack_.push({dir_id, PreprocDirectKind::IFDEF, Loc});

  // #ifdef macro IS defined -> branch is true
  bool evaluated = (bool)MD;
  recordBranchEvaluation(dir_id, evaluated);

  extractDirectiveText(Loc, dir_id, PreprocDirectKind::IFDEF);
}

void PreprocessorProcessor::If(SourceLocation Loc, SourceRange ConditionRange,
                               ConditionValueKind ConditionValue) {
  int dir_id = processDirective(Loc, PreprocDirectKind::IF);
  branch_stack_.push({dir_id, PreprocDirectKind::IF, Loc});

  // Check if condition evaluated to true
  bool evaluated = (ConditionValue == CVK_True);
  recordBranchEvaluation(dir_id, evaluated);

  extractDirectiveText(Loc, dir_id, PreprocDirectKind::IF);
}

void PreprocessorProcessor::Elif(SourceLocation Loc, SourceRange ConditionRange,
                                ConditionValueKind ConditionValue,
                                SourceLocation IfLoc) {
  int dir_id = processDirective(Loc, PreprocDirectKind::ELIF);

  // Pop the previous branch (if/elif) and record pair
  if (!branch_stack_.empty()) {
    BranchInfo prev = branch_stack_.top();
    branch_stack_.pop();
    recordBranchPair(prev.directive_id, dir_id);
  }

  // Push this elif onto stack
  branch_stack_.push({dir_id, PreprocDirectKind::ELIF, Loc});

  // Record evaluation
  bool evaluated = (ConditionValue == CVK_True);
  recordBranchEvaluation(dir_id, evaluated);

  extractDirectiveText(Loc, dir_id, PreprocDirectKind::ELIF);
}

void PreprocessorProcessor::Else(SourceLocation Loc, SourceLocation IfLoc) {
  int dir_id = processDirective(Loc, PreprocDirectKind::ELSE);

  // Pop the previous branch (if/elif) and record pair
  if (!branch_stack_.empty()) {
    BranchInfo prev = branch_stack_.top();
    branch_stack_.pop();
    recordBranchPair(prev.directive_id, dir_id);
  }

  // Push else onto stack
  branch_stack_.push({dir_id, PreprocDirectKind::ELSE, Loc});

  extractDirectiveText(Loc, dir_id, PreprocDirectKind::ELSE);
}

void PreprocessorProcessor::Endif(SourceLocation Loc, SourceLocation IfLoc) {
  int dir_id = processDirective(Loc, PreprocDirectKind::ENDIF);

  // Pop the previous branch (if/elif/else) and record pair
  if (!branch_stack_.empty()) {
    BranchInfo prev = branch_stack_.top();
    branch_stack_.pop();
    recordBranchPair(prev.directive_id, dir_id);
  }

  extractDirectiveText(Loc, dir_id, PreprocDirectKind::ENDIF);
}

//////////////////////////////////////////////////////////////////////////////
// Include Directive Callbacks
//////////////////////////////////////////////////////////////////////////////

void PreprocessorProcessor::InclusionDirective(
    SourceLocation HashLoc, const Token &IncludeTok, StringRef FileName,
    bool IsAngled, CharSourceRange FilenameRange,
    OptionalFileEntryRef File,
    StringRef SearchPath, StringRef RelativePath,
    const Module *SuggestedModule, bool ModuleImported,
    SrcMgr::CharacteristicKind FileType) {

  // Determine include kind
  PreprocDirectKind kind;
  switch (IncludeTok.getIdentifierInfo()->getPPKeywordID()) {
    case tok::pp_include:
      kind = PreprocDirectKind::PLAIN_INCLUDE;
      break;
    case tok::pp_import:
      kind = PreprocDirectKind::OBJC_IMPORT;
      break;
    case tok::pp_include_next:
      kind = PreprocDirectKind::INCLUDE_NEXT;
      break;
    default:
      kind = PreprocDirectKind::PLAIN_INCLUDE;
      break;
  }

  int dir_id = processDirective(HashLoc, kind);
  extractDirectiveText(HashLoc, dir_id, kind);

  // Resolve and record included file
  if (File) {
    std::string included_path = File->getName().str();
    int file_id = resolveIncludeFile(included_path);

    Includes include = {dir_id, file_id};
    STG.insertClassObj(include);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Macro Definition Callbacks
//////////////////////////////////////////////////////////////////////////////

void PreprocessorProcessor::MacroDefined(const Token &MacroNameTok,
                                        const MacroDirective *MD) {
  SourceLocation Loc = MacroNameTok.getLocation();
  int dir_id = processDirective(Loc, PreprocDirectKind::DEFINE);
  extractDirectiveText(Loc, dir_id, PreprocDirectKind::DEFINE);
}

void PreprocessorProcessor::MacroUndefined(const Token &MacroNameTok,
                                          const MacroDefinition &MD,
                                          const MacroDirective *Undef) {
  SourceLocation Loc = MacroNameTok.getLocation();
  int dir_id = processDirective(Loc, PreprocDirectKind::UNDEF);
  extractDirectiveText(Loc, dir_id, PreprocDirectKind::UNDEF);
}

//////////////////////////////////////////////////////////////////////////////
// Other Directive Callbacks
//////////////////////////////////////////////////////////////////////////////

void PreprocessorProcessor::PragmaDirective(SourceLocation Loc,
                                           PragmaIntroducerKind Introducer) {
  int dir_id = processDirective(Loc, PreprocDirectKind::PRAGMA);
  extractDirectiveText(Loc, dir_id, PreprocDirectKind::PRAGMA);
}

void PreprocessorProcessor::PragmaMessage(SourceLocation Loc, StringRef Namespace,
                                         PragmaMessageKind Kind,
                                         StringRef Str) {
  // #pragma warning or #pragma message
  PreprocDirectKind kind = (Kind == PMK_Warning) ? PreprocDirectKind::WARNING
                                                  : PreprocDirectKind::ERROR;
  int dir_id = processDirective(Loc, kind);
  extractDirectiveText(Loc, dir_id, kind);
}

void PreprocessorProcessor::PragmaDebug(SourceLocation Loc, StringRef DebugType) {
  int dir_id = processDirective(Loc, PreprocDirectKind::PRAGMA);
  extractDirectiveText(Loc, dir_id, PreprocDirectKind::PRAGMA);
}

//////////////////////////////////////////////////////////////////////////////
// Helper Methods
//////////////////////////////////////////////////////////////////////////////

int PreprocessorProcessor::processDirective(SourceLocation Loc,
                                           PreprocDirectKind kind) {
  // Check cache first
  KeyType key = KeyGen::Preprocessor::makeKey(Loc, *preprocessor_);
  auto cached_id = SEARCH_PREPROC_CACHE(key);
  if (cached_id.has_value()) {
    return *cached_id;
  }

  // Generate ID and create directive
  int dir_id = GENID(Preprocdirect);

  // Record location (preprocessor directives are at a single location)
  LocIdPair *loc_pair = PROC_DEFT(Loc, Loc, ast_context_);

  Preprocdirect directive = {dir_id, static_cast<int>(kind), loc_pair->spec_id};
  STG.insertClassObj(directive);

  // Insert into cache
  INSERT_PREPROC_CACHE(key, dir_id);

  LOG_DEBUG << "Preprocessor directive kind=" << static_cast<int>(kind)
            << " id=" << dir_id << std::endl;

  return dir_id;
}

void PreprocessorProcessor::extractDirectiveText(SourceLocation Loc,
                                                int directive_id,
                                                PreprocDirectKind kind) {
  const SourceManager &SM = ast_context_->getSourceManager();

  // Get the start of the line containing the directive
  SourceLocation LineStart = SM.getExpansionLoc(Loc);

  // Try to get a reasonable amount of text (up to 512 characters)
  std::string full_text = Lexer::getSourceText(
      CharSourceRange::getTokenRange(LineStart, LineStart.getLocWithOffset(512)),
      SM, LangOptions()).str();

  // Truncate at newline
  size_t newline_pos = full_text.find('\n');
  if (newline_pos != std::string::npos) {
    full_text = full_text.substr(0, newline_pos);
  }

  // For #define, split into head (macro name) and body (macro value)
  std::string head = full_text;
  std::string body = "";

  if (kind == PreprocDirectKind::DEFINE) {
    // Find the first space after #define
    size_t define_pos = head.find("#define");
    if (define_pos != std::string::npos) {
      size_t name_start = define_pos + 7; // skip "#define"
      // Skip whitespace
      while (name_start < head.length() && std::isspace(head[name_start])) {
        name_start++;
      }
      // Find the end of the macro name
      size_t name_end = name_start;
      while (name_end < head.length() && !std::isspace(head[name_end]) &&
             head[name_end] != '(') {
        name_end++;
      }
      // Head is directive + macro name
      head = head.substr(0, name_end);
      // Extract body (everything after name)
      size_t value_start = name_end;
      while (value_start < head.length() && std::isspace(head[value_start])) {
        value_start++;
      }
      if (value_start < full_text.length()) {
        body = full_text.substr(value_start);
      }
    }
  }

  Preproctext text = {directive_id, head, body};
  STG.insertClassObj(text);
}

void PreprocessorProcessor::recordBranchPair(int begin_id, int end_id) {
  Preprocpair pair = {begin_id, end_id};
  STG.insertClassObj(pair);

  LOG_DEBUG << "Branch pair: begin=" << begin_id << " end=" << end_id
            << std::endl;
}

void PreprocessorProcessor::recordBranchEvaluation(int branch_id, bool is_true) {
  branch_evaluation_[branch_id] = is_true;

  if (is_true) {
    Preproctrue true_branch = {branch_id};
    STG.insertClassObj(true_branch);
  } else {
    Preprocfalse false_branch = {branch_id};
    STG.insertClassObj(false_branch);
  }
}

int PreprocessorProcessor::resolveIncludeFile(const std::string &filename) {
  // For now, create a new file entry each time
  // TODO: Check if file already exists in database to avoid duplicates

  int file_id = GENID(File);
  std::string file_name = std::filesystem::path(filename).filename().string();

  File file = {file_id, file_name};
  STG.insertClassObj(file);

  return file_id;
}

std::string PreprocessorProcessor::getSourceText(CharSourceRange range) {
  const SourceManager &SM = ast_context_->getSourceManager();
  return Lexer::getSourceText(range, SM, LangOptions()).str();
}
