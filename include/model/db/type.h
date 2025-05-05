#ifndef _MODEL_TYPE_H_
#define _MODEL_TYPE_H_

#include <string>

enum class TypeType {
  BUILTIN_TYPE = 1,
  DERIVED_TYPE = 2,
  USER_TYPE = 3,
  FIXED_POINT_TYPE = 4,
  ROUTINE_TYPE = 5,
  PTR_TO_MEMBER = 6,
  DECL_TYPE = 7
};

enum class UserTypeKind {
  UNKNOWN_USERTYPE = 0,
  STRUCT = 1,
  CLASS = 2,
  UNION = 3,
  ENUM = 4,
  TYPEDEF = 5,
  TEMPLATE = 6, // Deprecated
  TEMPLATE_PARAMETER = 7,
  TEMPLATE_TEMPLATE_PARAMETER = 8,
  PROXY_CLASS = 9,
  OBJC_CLASS = 10,    // Deprecated
  OBJC_PROTOCOL = 11, // Deprecated
  OBJC_CATEGORY = 12, // Deprecated
  SCOPED_ENUM = 13,
  USING_ALIAS = 14,
  TEMPLATE_STRUCT = 15,
  TEMPLATE_CLASS = 16,
  TEMPLATE_UNION = 17
};

namespace DbModel {

struct Type {
  int id;
  int associate_id;
  int type;
  using KeyType = std::string;
};

struct UserType {
  int id;
  std::string name;
  int kind;
  using KeyType = std::string;
};

} // namespace DbModel

#endif // _MODEL_TYPE_H_
