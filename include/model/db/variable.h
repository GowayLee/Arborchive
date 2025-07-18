#ifndef _MODEL_VARIABLE_H_
#define _MODEL_VARIABLE_H_

#include <string>

// TO BE REMOVED: VarType enum - no longer needed for direct relationships
// enum class VarType {
//   LOCAL_SCOPE_VARIABLE = 1,
//   GLOBAL_VARIABLE = 2,
//   MEMBER_VARIABLE = 3
// };

// TO BE REMOVED: LocalScopeVarType enum - no longer needed for direct relationships
// enum class LocalScopeVarType { LOCAL_VARIABLE = 1, PARAMETER = 2 };

namespace DbModel {

// TO BE REMOVED: Variable struct - intermediary table no longer needed
struct Variable {
   int id;
   int associate_id;
   int type;
   using KeyType = std::string;
 };

// TO BE REMOVED: LocalScopeVar struct - intermediary table no longer needed
// truct LocalScopeVar {
//   int id;
//   int associate_id;
//   int type;
// };

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
