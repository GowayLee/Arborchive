#ifndef _MODEL_DECLRATION_H_
#define _MODEL_DECLRATION_H_

#include <string>

enum class DeclType {
  FUNCTION = 1,
  DECLARED_TYPE = 2,
  VARIABLE = 3,
  ENUM_CONSTANT = 4,
  FRIEND_DECL = 5,
  CONCEPT_TEMPLATE = 6
};

namespace DbModel {

struct Declaration {
  int id;
  int associated_id;
  int type;
};

struct FriendDecl {
  int id;
  int type_id;
  int decl_id;
  int location;
};

struct NamespaceDecl {
  int id;
  int namespace_id;
  int location;
  int bodylocation;
};

struct Using {
  int id;
  int element_id;
  int location;
  int kind;
};

struct UsingContainer {
  int parent;
  int child;
};

} // namespace DbModel

#endif // _MODEL_DECLRATION_H_
