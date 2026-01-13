#ifndef _MODEL_PREPROCESSOR_H_
#define _MODEL_PREPROCESSOR_H_

#include <string>

enum class PreprocDirectKind {
  IF = 0,
  IFDEF = 1,
  IFNDEF = 2,
  ELIF = 3,
  ELSE = 4,
  ENDIF = 5,
  PLAIN_INCLUDE = 6,
  DEFINE = 7,
  UNDEF = 8,
  LINE = 9,
  ERROR = 10,
  PRAGMA = 11,
  OBJC_IMPORT = 12,
  INCLUDE_NEXT = 13,
  MS_IMPORT = 14,
  ELIFDEF = 15,
  ELIFNDEF = 16,
  WARNING = 18
};

namespace DbModel {

struct Preprocdirect {
  int id;
  int kind;
  int location;
  using KeyType = std::string;
};

struct Preprocpair {
  int begin;
  int elseelifend;
};

struct Preproctrue {
  int branch;
};

struct Preprocfalse {
  int branch;
};

struct Preproctext {
  int id;
  std::string head;
  std::string body;
};

struct Includes {
  int id;
  int included;
};

} // namespace DbModel

#endif // _MODEL_PREPROCESSOR_H_
