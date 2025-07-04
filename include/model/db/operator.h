#ifndef _MODEL_OPERATOR_H_
#define _MODEL_OPERATOR_H_

#include <string>

// Operator Types

enum class OpExprType {
  _UNKNOWN_ = -1,
  UN_OP_EXPR = 1,
  BIN_OP_EXPR = 2,
  ASSIGN_EXPR = 3,
  CONDITIONALEXPR = 4
};

// 1. Unary Operator
enum class UnOpExprType {
  _UNKNOWN_ = -1,
  ADDRESS_OF = 1,
  INDIRECT = 2,
  UN_ARITH_OP_EXPR = 3,
  UN_BITEWISE_OP_EXPR = 4,
  BUILTINADDRESSOF = 5,
  VEC_FILL = 6,
  UN_LOG_OP_EXPR = 7,
  CO_AWAIT = 8,
  CO_YEILD = 9
};

// 1.1 Unary Arithmetic
enum class UnArithOpExprType {
  _UNKNOWN_ = -1,
  ARITHNEGEXPR = 1,
  UNARYPLUSEXPR = 2,
  CONJUGATION = 3,
  REALPARTEXPR = 4,
  IMAGPARTEXPR = 5,
  CREMENT_EXPR = 6
};

// 1.1.1 Crement Expr
enum class CrementExpr {
  _UNKNOWN_ = -1,
  INCREMENT_EXPR = 1,
  DECREMENT_EXPR = 2
};

// 1.2 Unary Bitwise
enum class UnBitwiseOpExprType { COMPLEMENTEXPR = 1 };

// 1.3 Unary Logical
enum class UnLogOpExprType { NOTEXPR = 1 };

// 2. Binary Operator
enum class BinOpExprType {
  _UNKNOWN_ = -1,
  BIN_ARITH_OP_EXPR = 1,
  BIN_BITWISE_OP_EXPR = 2,
  CMP_OP_EXPR = 3,
  BIN_LOG_OP_EXPR = 4
};

// 2.1 Binary Arithmetic
enum class BinArithOpExprType {
  _UNKNOWN_ = -1,
  ADDEXPR = 1,
  SUBEXPR = 2,
  MULEXPR = 3,
  DIVEEXPR = 4,
  REMEXPR = 5,
  JMULEXPR = 6,
  JDIVEXPR = 7,
  FJADDEXPR = 8,
  JFADDEXPR = 9,
  FJSUBEXPR = 10,
  JFSUBEXPR = 11,
  MINEXPR = 12,
  MAXEXPR = 13,
  P_ARITH_OP_EXPR = 14
};

// 2.1.1 Prefix Arithmetic
enum class POpExprType {
  _UNKNOWN_ = -1,
  PADDEXPR = 1,
  PSUBEXPR = 2,
  PDIFFEXPR = 3
};

// 2.2 Binary Bitwise
enum class BinBitwiseOpExprType {
  _UNKNOWN_ = -1,
  LSHIFTEXPR = 1,
  RSHIFTEXPR = 2,
  ANDEXPR = 3,
  OREXPR = 4,
  XOREXPR = 5
};

// 2.3 Comparison
enum class CmpOpExprType {
  _UNKNOWN_ = -1,
  EQ_OP_EXPR = 1,
  REL_OP_EXPR = 2
};

// 2.3.1 Equality
enum class EqOpExprType { _UNKNOWN_ = -1, EQEXPR = 1, NEEXPR = 2 };

// 2.3.2 Relational
enum class RelOpExprType {
  _UNKNOWN_ = -1,
  GTEXPR = 1,
  LTEXPR = 2,
  GEEXPR = 3,
  LEEXPR = 4,
  SPACESHIPEXPR = 5
};

// 2.4 Binary Logical
enum class BinLogOpExprType {
  _UNKNOWN_ = -1,
  ANDLOGICALEXPR = 1,
  ORLOGICALEXPR = 2
};

namespace DbModel {

struct OpExpr {
  int id;
  int associate_id;
  int type;
};

struct UnOpExpr {
  int id;
  int associate_id;
  int type;
};

struct UnArithOpExpr {
  int id;
  int associate_id;
  int type;
};

struct UnBitwiseOpExpr {
  int id;
  int associate_id;
  int type;
};

struct UnLogOpExpr {
  int id;
  int associate_id;
  int type;
};

struct BinOpExpr {
  int id;
  int associate_id;
  int type;
};

} // namespace DbModel

#endif // _MODEL_OPERATOR_H_
