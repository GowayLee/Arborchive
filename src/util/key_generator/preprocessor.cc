#include "util/key_generator/preprocessor.h"

namespace KeyGen {

namespace Preprocessor {

KeyType makeKey(const std::string &filename, unsigned line, unsigned column,
                int directive_kind, const std::string &extra_tag) {
  std::string normalized_filename = filename.empty() ? "<invalid>" : filename;
  KeyType key = normalized_filename + ":" + std::to_string(line) + ":" +
                std::to_string(column) + ":" + std::to_string(directive_kind);
  if (!extra_tag.empty())
    key += ":" + extra_tag;
  return key;
}

} // namespace Preprocessor

} // namespace KeyGen
