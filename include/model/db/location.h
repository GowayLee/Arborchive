#ifndef _MODEL_LOCATION_H_
#define _MODEL_LOCATION_H_

#include <string>

enum class LocationType { DEFAULT = 1, STMT = 2, EXPR = 3 };

namespace DbModel {

struct Location {
  int id;
  int associated_id;
};

struct LocationDefault {
  int id;
  int container;
  int start_line;
  int start_column;
  int end_line;
  int end_column;
};

struct LocationStmt {
  int id;
  int container;
  int start_line;
  int start_column;
  int end_line;
  int end_column;
};

struct LocationExpr {
  int id;
  int container;
  int start_line;
  int start_column;
  int end_line;
  int end_column;
};

} // namespace DbModel

#endif // _MODEL_LOCATION_H_
