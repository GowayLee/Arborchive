#ifndef _MODEL_SPECIFIERS_H_
#define _MODEL_SPECIFIERS_H_

#include <string>

namespace DbModel {

struct Specifier {
  int id;
  std::string str;
  using KeyType = std::string;
};

struct TypeSpecifiers {
  int type_id;
  int spec_id;
};

struct FunSpecifiers {
  int func_id;
  int spec_id;
};

struct VarSpecifiers {
  int var_id;
  int spec_id;
};

} // namespace DbModel

#endif // _MODEL_SPECIFIERS_H_
