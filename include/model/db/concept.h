#ifndef _MODEL_CONCEPT_H_
#define _MODEL_CONCEPT_H_

#include <string>

namespace DbModel {

struct ConceptTemplate {
  int id;
  std::string name;
  int location;
  using KeyType = std::string;
};

struct ConceptInstantiation {
  int to;
  int from;
};

struct ConceptTemplateArgument {
  int concept_id;
  int index;
  int arg_type;
};

struct IsTypeConstraint {
  int concept_id;
};

struct ConceptTemplateArgumentValue {
  int concept_id;
  int index;
  int arg_value;
};

} // namespace DbModel

#endif // _MODEL_CONCEPT_H_
