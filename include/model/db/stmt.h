#ifndef _MODEL_STMT_H_
#define _MODEL_STMT_H_

#include <string>

enum class StmtKind {
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

// enum class ForType { FOR = 1, RANGE_BASED_FOR = 2 };

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

struct ForInit {
  int for_stmt;
  int init_id;
};

struct ForCond {
  int for_stmt;
  int condition_id;
};

struct ForUpdate {
  int for_stmt;
  int update_id;
};

struct ForBody {
  int for_stmt;
  int body_id;
};

struct WhileBody {
  int while_stmt;
  int body_id;
};

struct DoBody {
  int do_stmt;
  int body_id;
};

struct SwitchInit {
  int switch_stmt;
  int init_id;
};

struct SwitchCase {
  int switch_stmt;
  int index;
  int case_id;
};

struct SwitchBody {
  int switch_stmt;
  int body_id;
};

} // namespace DbModel

#endif // _MODEL_STMT_H_
