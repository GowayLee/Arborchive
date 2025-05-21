#ifndef _MODEL_VARIABLE_H_
#define _MODEL_VARIABLE_H_

#include <string>

enum class VarType {
  LOCAL_SCOPE_VARIABLE = 1,
  GLOBAL_VARIABLE = 2,
  MEMBER_VARIABLE = 3
};

enum class LocalScopeVarType { LOCAL_VARIABLE = 1, PARAMETER = 2 };

namespace DbModel {

struct Variable {
  int id;
  int associate_id;
  int type;
};

struct LocalScopeVar {
  int id;
  int associate_id;
  int type;
};

struct LocalVar {
  int id;
  int type_id;
  std::string name;
};

struct Parameter {
  int id;
  int function;
  int index;
  int type_id;
};

struct GlobalVar {
  int id;
  int type_id;
  std::string name;
};

struct MemberVar {
  int id;
  int type_id;
  std::string name;
};

struct VarDecl {
  int id;
  int variable;
  int type_id;
  std::string name;
  int location;
};

struct VarDef {
  int id;
};

struct VarSpecialized {
  int id;
};

struct VarDeclSpec {
  int id;
  std::string name;
};

struct IsStructuredBinding {
  int id;
};

struct VarRequire {
  int id;
  int constraint;
};
} // namespace DbModel

#endif // _MODEL_VARIABLE_H_
