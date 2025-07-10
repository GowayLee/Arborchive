#include "util/key_generator/values.h"
#include <sstream>

namespace KeyGen {

namespace Values {
KeyType makeKey(const std::string &value) {
  std::ostringstream oss;
  oss << "val-" << value;
  return oss.str();
}
} // namespace Values

namespace ValueText {
KeyType makeKey(const std::string &text) {
  std::ostringstream oss;
  oss << "txt-" << text;
  return oss.str();
}
} // namespace ValueText

} // namespace KeyGen