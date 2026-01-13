#include "util/key_generator/preprocessor.h"
#include <clang/Basic/SourceManager.h>

using namespace clang;

namespace KeyGen {

namespace Preprocessor {

KeyType makeKey(SourceLocation Loc, class Preprocessor &PP) {
  const SourceManager &SM = PP.getSourceManager();

  // Get file path, line, and column for unique key
  std::string filename = SM.getFilename(Loc).str();
  unsigned line = SM.getSpellingLineNumber(Loc);
  unsigned column = SM.getSpellingColumnNumber(Loc);

  return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
}

} // namespace Preprocessor

} // namespace KeyGen
