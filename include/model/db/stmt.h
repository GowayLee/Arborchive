#ifndef _MODEL_STMT_H_
#define _MODEL_STMT_H_

#include <string>

enum class StmtType {
  EXPR = 1,
  IF = 2,
  WHILE = 3,
  GOTO = 4,
  LABEL = 5,
  RETURN = 6,
  BLOCK = 7,
  END_TEST_WHILE = 8,
  FOR = 9,
  SWITCH_CASE = 10,
  SWITCH = 11,
  ASM = 13,
  TRY_BLOCK = 15,
  MICROSOFT_TRY = 16,
  DECL = 17,
  SET_VLA_SIZE = 18,
  VLA_DECL = 19,
  ASSIGNED_GOTO = 25,
  EMPTY = 26,
  CONTINUE = 27,
  BREAK = 28,
  RANGE_BASED_FOR = 29,
  HANDLER = 33,
  CONSTEXPR_IF = 35,
  CO_RETURN = 37,
  CONSTEVAL_IF = 38,
  NOT_CONSTEVAL_IF = 39
};

namespace DbModel {

struct Stmt {
  int id;
  int kind;
  int location;
  using KeyType = std::string;
};

struct IfInit {
  int if_stmt;
  int init_id;
};

struct IfThen {
  int if_stmt;
  int then_id;
};

struct IfElse {
  int if_stmt;
  int else_id;
};

} // namespace DbModel

#endif // _MODEL_STMT_H_
