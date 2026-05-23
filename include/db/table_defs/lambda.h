#ifndef _TABLE_DEFS_LAMBDA_H_
#define _TABLE_DEFS_LAMBDA_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/lambda.h"

using namespace sqlite_orm;

namespace LambdaTableFn {

// clang-format off
inline auto lambdas() {
  return make_table(
      "lambdas",
      make_column("expr", &DbModel::Lambda::expr, primary_key()),
      make_column("default_capture", &DbModel::Lambda::default_capture),
      make_column("has_explicit_return_type",
                  &DbModel::Lambda::has_explicit_return_type));
}

inline auto lambda_capture() {
  return make_table(
      "lambda_capture",
      make_column("id", &DbModel::LambdaCapture::id, primary_key()),
      make_column("lambda", &DbModel::LambdaCapture::lambda),
      make_column("index", &DbModel::LambdaCapture::index),
      make_column("field", &DbModel::LambdaCapture::field),
      make_column("captured_by_reference",
                  &DbModel::LambdaCapture::captured_by_reference),
      make_column("is_implicit", &DbModel::LambdaCapture::is_implicit),
      make_column("location", &DbModel::LambdaCapture::location));
}
// clang-format on

} // namespace LambdaTableFn

#endif // _TABLE_DEFS_LAMBDA_H_
