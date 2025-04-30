#ifndef _MODEL_FUNCTION_H_
#define _MODEL_FUNCTION_H_

#include <string>

enum class FuncType {
  NORM_FUNC = 1,
  CONSTRUCTOR = 2,
  DESTRUCTOR = 3,
  CONVERSION_FUNC = 4,
  OPERATOR = 5,
  BUILDIN_FUNC = 6,
  USER_DEFINED_LITERAL = 7,
  DEDUCTION_GUIDE = 8,
};

namespace DbModel {

struct Function {
  int id;
  std::string name;
  int kind;
};

struct FunDecl {
  int id;
  int function;
  int type_id;
  std::string name;
  int location;
};

struct FunDef {
  int id;
};

} // namespace DbModel

#endif // _MODEL_FUNCTION_H_
