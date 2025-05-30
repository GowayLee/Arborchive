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

enum class BuiltinTypeKind {
  ERRORTYPE = 1,
  UNKOWNTYPE = 2,
  VOID = 3,
  BOOLEAN = 4,
  CHAR = 5,
  UNSIGNED_CHAR = 6,
  SIGNED_CHAR = 7,
  SHORT = 8,
  UNSIGNED_SHORT = 9,
  SIGNED_SHORT = 10,
  INT = 11,
  UNSIGNED_INT = 12,
  SIGNED_INT = 13,
  LONG = 14,
  UNSIGNED_LONG = 15,
  SIGNED_LONG = 16,
  LONG_LONG = 17,
  UNSIGNED_LONG_LONG = 18,
  SIGNED_LONG_LONG = 19,
  // Microsoft-specific_int8 = 20
  // Microsoft-specific_int16 = 21
  // Microsoft-specific_int32 = 22
  // Microsoft-specific_int64 = 23
  FLOAT = 24,
  DOUBLE = 25,
  LONG_DOUBLE = 26,
  COMPLEX_FLOAT = 27,
  COMPLEX_DOUBLE = 28,
  COMPLEX_LONG_DOUBLE = 29,
  IMAGINARY_FLOAT = 30,
  IMAGINARY_DOUBLE = 31,
  IMAGINARY_LONG_DOUBLE = 32,
  WCHAR_T = 33,
  DECLTYPE_NULLPTR = 34,
  INT128 = 35,
  UNSIGNED_INT128 = 36,
  SIGNED_INT128 = 37,
  FLOAT128 = 38,
  COMPLEX_FLOAT128 = 39,
  DECIMAL32 = 40,
  DECIMAL64 = 41,
  DECIMAL128 = 42,
  CHAR16_T = 43,
  CHAR32_T = 44,
  STD_FLOAT32 = 45,
  FLOAT32X = 46,
  STD_FLOAT64 = 47,
  FLOAT64X = 48,
  STD_FLOAT128 = 49,
  // _FLOAT128X = 50,
  CHAR8_T = 51,
  FLOAT16 = 52,
  COMPLEX_FLOAT16 = 53,
  FP16 = 54,
  STD_BFLOAT16 = 55,
  STD_FLOAT16 = 56,
  COMPLEX_STD_FLOAT32 = 57,
  COMPLEX_FLOAT32X = 58,
  COMPLEX_STD_FLOAT64 = 59,
  COMPLEX_FLOAT64X = 60,
  COMPLEX_STD_FLOAT128 = 61
};

enum class DerivedTypeKind {
  POINTER = 1,
  REFERENCE = 2,
  TYPE_WITH_SPECIFIERS = 3,
  ARRAY = 4,
  GNU_VECTOR = 5,
  ROUTINEPTR = 6,
  ROUTINEREFERENCE = 7,
  RVALUE_REFERENCE = 8,
  // ... 9 type_conforming_to_protocols deprecated
  BLOCK = 10
};

namespace DbModel {

struct Type {
  int id;
  int associate_id;
  int type;
  using KeyType = std::string;
};

struct TypeDecl {
  int id;
  int type_id;
  int location;
};

struct TypeDef {
  int id;
};

struct TypeDeclTop {
  int type_decl;
};

struct BuiltinType_ {
  int id;
  std::string name;
  int kind;
  int size;
  int sign;
  int alignment;
};

struct DerivedType {
  int id;
  std::string name;
  int kind;
  int type_id;
};

struct UserType {
  int id;
  std::string name;
  int kind;
  using KeyType = std::string;
};

struct RoutineType {
  int id;
  int return_type;
};

struct RoutineTypeArg {
  int routine;
  int index;
  int type_id;
};

struct PtrToMember {
  int id;
  int type_id;
  int class_id;
};

struct DeclType {
  int id;
  int expr;
  int base_type;
  bool parentheses_would_change_meaning;
};

} // namespace DbModel

#endif // _MODEL_TYPE_H_
