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

struct FuncRetType {
  int id;
  int return_type;
};

struct PureFuncs {
  int id;
};

struct FuncDeleted {
  int id;
};

struct FuncDefaulted {
  int id;
};

struct FuncPrototyped {
  int id;
};

struct FunSpecialized {
  int id;
};

struct FunImplicit {
  int id;
};

struct FuncEntryPt {
  int id;
  int entry_point;
};

struct DeductionGuideForClass {
  int id;
  int class_template;
};

struct FunDeclThrow {
  int fun_decl;
  int index;
  int type_id;
};

struct FunDeclEmptyThrow {
  int fun_decl;
};

struct FunDeclNoexcept {
  int fun_decl;
  int constant;
};

struct FunDeclEmptyNoexcept {
  int fun_decl;
};

} // namespace DbModel

#endif // _MODEL_FUNCTION_H_
