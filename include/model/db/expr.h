#ifndef _MODEL_EXPR_H_
#define _MODEL_EXPR_H_

#include <string>

enum class ExprKind {
  // clang-format off
  _UNKNOWN_ = -1,
  ERROREXPR = 1,
  ADDRESS_OF = 2,   // & AddressOfExpr
  REFERENCE_TO = 3, // ReferenceToExpr (implicit?)
  INDIRECT = 4,     // * PointerDereferenceExpr
  REF_INDIRECT = 5, // ReferenceDereferenceExpr (implicit?)
  // ...
  ARRAY_TO_POINTER = 8,        // (???)
  VACUOUS_DESTRUCTOR_CALL = 9, // VacuousDestructorCall
  // ...
  ASSUME = 11, // Microsoft
  PAREXPR = 12,
  ARITHNEGEXPR = 13,
  UNARYPLUSEXPR = 14,
  COMPLEMENTEXPR = 15,
  NOTEXPR = 16,
  CONJUGATION = 17,  // GNU ~ operator
  REALPARTEXPR = 18, // GNU __real
  IMAGPARTEXPR = 19, // GNU __imag
  POSTINCREXPR = 20,
  POSTDECREXPR = 21,
  PREINCREXPR = 22,
  PREDECREXPR = 23,
  CONDITIONALEXPR = 24,
  ADDEXPR = 25,
  SUBEXPR = 26,
  MULEXPR = 27,
  DIVEXPR = 28,
  REMEXPR = 29,
  JMULEXPR = 30,  // C99 mul imaginary
  JDIVEXPR = 31,  // C99 div imaginary
  FJADDEXPR = 32, // C99 add real + imaginary
  JFADDEXPR = 33, // C99 add imaginary + real
  FJSUBEXPR = 34, // C99 sub real - imaginary
  JFSUBEXPR = 35, // C99 sub imaginary - real
  PADDEXPR = 36,  // pointer add (pointer + int or int + pointer)
  PSUBEXPR = 37,  // pointer sub (pointer - integer)
  PDIFFEXPR = 38, // difference between two pointers
  LSHIFTEXPR = 39,
  RSHIFTEXPR = 40,
  ANDEXPR = 41,
  OREXPR = 42,
  XOREXPR = 43,
  EQEXPR = 44,
  NEEXPR = 45,
  GTEXPR = 46,
  LTEXPR = 47,
  GEEXPR = 48,
  LEEXPR = 49,
  MINEXPR = 50, // GNU minimum
  MAXEXPR = 51, // GNU maximum
  ASSIGNEXPR = 52,
  ASSIGNADDEXPR = 53,
  ASSIGNSUBEXPR = 54,
  ASSIGNMULEXPR = 55,
  ASSIGNDIVEXPR = 56,
  ASSIGNREMEXPR = 57,
  ASSIGNLSHIFTEXPR = 58,
  ASSIGNRSHIFTEXPR = 59,
  ASSIGNANDEXPR = 60,
  ASSIGNOREXPR = 61,
  ASSIGNXOREXPR = 62,
  ASSIGNPADDEXPR = 63, // assign pointer add
  ASSIGNPSUBEXPR = 64, // assign pointer sub
  ANDLOGICALEXPR = 65,
  ORLOGICALEXPR = 66,
  COMMAEXPR = 67,
  SUBSCRIPTEXPR = 68, // access to member of an array, e.g., a[5]
  // OBJC_SUBSRCIPTEXPR = 69, // deprecated
  // CMDACCESS = 70, // deprecated
  // ...
  VIRTFUNPTREXPR = 73,
  CALLEXPR = 74,
  // MSGEXPR_NORMAL = 75, // deprecated
  // MSGEXPR_SUPER = 76, // deprecated
  // ATSELECTOREXPR = 77, // deprecated
  // ATPROTOCOLEXPR = 78, // deprecated
  VASTARTEXPR = 79,
  VAARGEXPR = 80,
  VAENDEXPR = 81,
  VACOPYEXPR = 82,
  // ATENCODEEXPR = 83, // deprecated
  VARACCESS = 84,
  THISACCESS = 85,
  // OBJC_BOX_EXPR = 86, // deprecated
  NEW_EXPR = 87,
  DELETE_EXPR = 88,
  THROW_EXPR = 89,
  CONDITION_DECL =
      90, // a variable declared in a condition, e.g., if(int x = y > 2)
  BRACED_INIT_LIST = 91,
  TYPE_ID = 92,
  RUNTIME_SIZEOF = 93,
  RUNTIME_ALIGNOF = 94,
  SIZEOF_PACK = 95,
  EXPR_STMT = 96, // GNU extension
  ROUTINEEXPR = 97,
  TYPE_OPERAND = 98, // used to access a type in certain contexts (haven't found any examples yet....)
  OFFSETOFEXPR = 99, // offsetof ::= type and field
  HASASSIGNEXPR = 100,    // __has_assign ::= type
  HASCOPYEXPR = 101,      // __has_copy ::= type
  HASNOTHROWASSIGN = 102, // __has_nothrow_assign ::= type
  HASNOTHROWCONSTR = 103, // __has_nothrow_constructor ::= type
  HASNOTHROWCOPY = 104,   // __has_nothrow_copy ::= type
  HASTRIVIALASSIGN = 105, // __has_trivial_assign ::= type
  HASTRIVIALCONSTR = 106, // __has_trivial_constructor ::= type
  HASTRIVIALCOPY = 107,   // __has_trivial_copy ::= type
  HASUSERDESTR = 108,     // __has_user_destructor ::= type
  HASVIRTUALDESTR = 109,  // __has_virtual_destructor ::= type
  ISABSTRACTEXPR = 110,   // __is_abstract ::= type
  ISBASEOFEXPR = 111,     // __is_base_of ::= type type
  ISCLASSEXPR = 112,      // __is_class ::= type
  ISCONVTOEXPR = 113,     // __is_convertible_to ::= type type
  ISEMPTYEXPR = 114,      // __is_empty ::= type
  ISENUMEXPR = 115,       // __is_enum ::= type
  ISPODEXPR = 116,        // __is_pod ::= type
  ISPOLYEXPR = 117,       // __is_polymorphic ::= type
  ISUNIONEXPR = 118,      // __is_union ::= type
  TYPESCOMPEXPR = 119,    // GNU __builtin_types_compatible ::= type type
  INTADDREXPR = 120, // frontend internal builtin, used to implement offsetof
  // ...
  HASTRIVIALDESTRUCTOR = 122, // __has_trivial_destructor ::= type
  LITERAL = 123,
  UUIDOF = 124,
  AGGREGATELITERAL = 127,
  DELETE_ARRAY_EXPR = 128,
  NEW_ARRAY_EXPR = 129,
  // OBJC_ARRAY_LITERAL = 130, // deprecated
  // OBJC_DICTIONARY_LITERAL, // deprecated
  FOLDEXPR = 132,
  // ...
  CTORDIRECTINIT = 200,
  CTORVIRTUALINIT = 201,
  CTORFIELDINIT = 202,
  CTORDELEGATINGINIT = 203,
  DTORDIRECTDESTRUCT = 204,
  DTORVIRTUALDESTRUCT = 205,
  DTORFIELDDESTRUCT = 206,
  // ...
  STATIC_CAST = 210,
  REINTERPRET_CAST = 211,
  CONST_CAST = 212,
  DYNAMIC_CAST = 213,
  C_STYLE_CAST = 214,
  LAMBDAEXPR = 215,
  PARAM_REF = 216,
  NOOPEXPR = 217,
  // ...
  ISTRIVIALLYCONSTRUCTIBLEEXPR = 294,
  ISDESTRUCTIBLEEXPR = 295,
  ISNOTHROWDESTRUCTIBLEEXPR = 296,
  ISTRIVIALLYDESTRUCTIBLEEXPR = 297,
  ISTRIVIALLYASSIGNABLEEXPR = 298,
  ISNOTHROWASSIGNABLEEXPR = 299,
  ISTRIVIALEXPR = 300,
  ISSTANDARDLAYOUTEXPR = 301,
  ISTRIVIALLYCOPYABLEEXPR = 302,
  ISLITERALTYPEEXPR = 303,
  HASTRIVIALMOVECONSTRUCTOREXPR = 304,
  HASTRIVIALMOVEASSIGNEXPR = 305,
  HASNOTHROWMOVEASSIGNEXPR = 306,
  ISCONSTRUCTIBLEEXPR = 307,
  ISNOTHROWCONSTRUCTIBLEEXPR = 308,
  HASFINALIZEREXPR = 309,
  ISDELEGATEEXPR = 310,
  ISINTERFACECLASSEXPR = 311,
  ISREFARRAYEXPR = 312,
  ISREFCLASSEXPR = 313,
  ISSEALEDEXPR = 314,
  ISSIMPLEVALUECLASSEXPR = 315,
  ISVALUECLASSEXPR = 316,
  ISFINALEXPR = 317,
  NOEXCEPTEXPR = 319,
  BUILTINSHUFFLEVECTOR = 320,
  BUILTINCHOOSEEXPR = 321,
  BUILTINADDRESSOF = 322,
  VEC_FILL = 323,
  BUILTINCONVERTVECTOR = 324,
  BUILTINCOMPLEX = 325,
  SPACESHIPEXPR = 326,
  CO_AWAIT = 327,
  CO_YIELD = 328,
  TEMP_INIT = 329,
  ISASSIGNABLE = 330,
  ISAGGREGATE = 331,
  HASUNIQUEOBJECTREPRESENTATIONS = 332,
  BUILTINBITCAST = 333,
  BUILTINSHUFFLE = 334,
  BLOCKASSIGNEXPR = 335,
  ISSAME = 336,
  ISFUNCTION = 337,
  ISLAYOUTCOMPATIBLE = 338,
  ISPOINTERINTERCONVERTIBLEBASEOF = 339,
  ISARRAY = 340,
  ARRAYRANK = 341,
  ARRAYEXTENT = 342,
  ISARITHMETIC = 343,
  ISCOMPLETETYPE = 344,
  ISCOMPOUND = 345,
  ISCONST = 346,
  ISFLOATINGPOINT = 347,
  ISFUNDAMENTAL = 348,
  ISINTEGRAL = 349,
  ISLVALUEREFERENCE = 350,
  ISMEMBERFUNCTIONPOINTER = 351,
  ISMEMBEROBJECTPOINTER = 352,
  ISMEMBERPOINTER = 353,
  ISOBJECT = 354,
  ISPOINTER = 355,
  ISREFERENCE = 356,
  ISRVALUEREFERENCE = 357,
  ISSCALAR = 358,
  ISSIGNED = 359,
  ISUNSIGNED = 360,
  ISVOID = 361,
  ISVOLATILE = 362,
  REUSEEXPR = 363,
  ISTRIVIALLYCOPYASSIGNABLE = 364,
  ISASSIGNABLENOPRECONDITIONCHECK = 365,
  REFERENCEBINDSTOTEMPORARY = 366,
  ISSAMEAS = 367,
  BUILTINHASATTRIBUTE = 368,
  ISPOINTERINTERCONVERTIBLEWITHCLASS = 369,
  BUILTINISPOINTERINTERCONVERTIBLEWITHCLASS = 370,
  ISCORRESPONDINGMEMBER = 371,
  BUILTINISCORRESPONDINGMEMBER = 372,
  ISBOUNDEDARRAY = 373,
  ISUNBOUNDEDARRAY = 374,
  ISREFERENCEABLE = 375,
  ISNOTHROWCONVERTIBLE = 378,
  REFERENCECONSTRUCTSFROMTEMPORARY = 379,
  REFERENCECONVERTSFROMTEMPORARY = 380,
  ISCONVERTIBLE = 381,
  ISVALIDWINRTTYPE = 382,
  ISWINCLASS = 383,
  ISWININTERFACE = 384,
  ISTRIVIALLYEQUALITYCOMPARABLE = 385,
  ISSCOPEDENUM = 386,
  ISTRIVIALLYRELOCATABLE = 387,
  DATASIZEOF = 388,
  C11_GENERIC = 389,
  REQUIRES_EXPR = 390,
  NESTED_REQUIREMENT = 391,
  COMPOUND_REQUIREMENT = 392,
  CONCEPT_ID = 393,
  // clang-format on
};

enum class IsCallKind {
  MBRCALLEXPR = 1,
  MBRPTRCALLEXPR = 2,
  MBRPTRMBRCALLEXPR = 3,
  PTRMBRPTRMBRCALLEXPR = 4,
  MBRREADEXPR = 5,
  MBRPTRREADEXPR = 6,
  MBRPTRMBRREADEXPR = 7,
  MBRPTRMBRPTRREADEXPR = 8,
  STATICMBRREADEXPR = 9,
  STATICMBRPTRREADEXPR = 10
};

namespace DbModel {

struct Expr {
  int id;
  int kind;
  int location;
  using KeyType = std::string;
};

struct FunBind {
  int expr;
  int fun;
};

struct IsCall {
  int caller;
  int kind;
};

struct VarBind {
  int expr;
  int var;
};

struct Values {
  int id;
  std::string str;
  using KeyType = std::string;
};

struct ValueText {
  int id;
  std::string text;
  using KeyType = std::string;
};

struct ValueBind {
  int val;
  int expr;
};

} // namespace DbModel

#endif // _MODEL_EXPR_H_
