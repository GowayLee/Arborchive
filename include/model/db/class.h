#ifndef _MODEL_CLASS_H_
#define _MODEL_CLASS_H_

#include <string>

enum class MemberType {
  MEMBERVARIABLE = 1,
  FUNCTION = 2,
  DECLAREDTYPE = 3,
  ENUMCONSTANT = 4
};

namespace DbModel {

struct Member {
  int id;
  int associated_id;
  int type;
};

} // namespace DbModel

#endif // _MODEL_CLASS_H_
