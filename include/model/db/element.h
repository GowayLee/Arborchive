#ifndef _MODEL_ELEMENT_H_
#define _MODEL_ELEMENT_H_

#include <string>

enum class ElementType {
  FUNCTION = 1,
  STMT_BLOCK = 2,
  REQUIRES_EXPR = 3,
};

namespace DbModel {

struct ParameterizedElement {
  int id;
  int associate_id;
  int type;
  using KeyType = std::string;
};

} // namespace DbModel

#endif // _MODEL_ELEMENT_H_
