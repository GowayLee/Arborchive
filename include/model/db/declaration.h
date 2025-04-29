#ifndef _MODEL_DECLRATION_H_
#define _MODEL_DECLRATION_H_

#include <string>

enum class DeclType {
  DECLARED_TYPE = 1,
  VARIABLE = 2,
  ENUM_CONSTANT = 3,
  FRIEND_DECL = 4,
  CONCEPT_TEMPLATE = 5
};

namespace DbModel {

struct Declaration {
  int id;
  int associated_id;
  int type;
};

} // namespace DbModel

#endif // _MODEL_DECLRATION_H_
