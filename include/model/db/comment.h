#ifndef _MODEL_COMMENT_H_
#define _MODEL_COMMENT_H_

#include <string>

namespace DbModel {

struct Comment {
  int id;
  std::string contents;
  int location;
};

struct CommentBinding {
  int id;
  int element;
};

} // namespace DbModel

#endif // _MODEL_COMMENT_H_
