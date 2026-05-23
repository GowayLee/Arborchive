#ifndef _MODEL_LAMBDA_H_
#define _MODEL_LAMBDA_H_

#include <string>

namespace DbModel {

struct Lambda {
  int expr;
  std::string default_capture;
  bool has_explicit_return_type;
};

struct LambdaCapture {
  int id;
  int lambda;
  int index;
  int field;
  bool captured_by_reference;
  bool is_implicit;
  int location;
};

} // namespace DbModel

#endif // _MODEL_LAMBDA_H_
