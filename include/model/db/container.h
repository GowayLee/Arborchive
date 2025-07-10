#ifndef _MODEL_CONTAINER_H_
#define _MODEL_CONTAINER_H_

#include <string>

enum class ContainerType { File = 0, Folder = 1 };

namespace DbModel {

struct Container {
  int id;
  int associated_id;
  int type;
};

struct File {
  int id;
  std::string name;
};

struct Folder {
  int id;
  std::string name;
};

struct Namespace {
  int id;
  std::string name;
};

struct NamespaceInline {
  int id;
};

struct NamespaceMember {
  int parentid;
  int memberid;
};

} // namespace DbModel

#endif // _MODEL_CONTAINER_H_