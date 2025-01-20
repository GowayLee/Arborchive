#ifndef _ASTNODEINFO_H
#define _ASTNODEINFO_H

#include <string>
#include <vector>

struct Location {
  std::string filename;
  unsigned line;
  unsigned column;

  std::string toString() const;
};

/*----------------------------.
|  Data Structs for AST Nodes |
`____________________________*/

struct FunctionDecl {
  std::string name;
  std::string returnType;
  std::vector<std::pair<std::string, std::string>> parameters;
  bool isDefinition;
  Location location;
};

struct CallExpr {
  std::string functionName;
  std::vector<std::string> arguments;
  Location location;
};

struct VarDecl {
  std::string name;
  std::string type;
  std::string initValue;
  Location location;
};

struct DeclRefExpr {
  std::string name;
  Location location;
};

#endif // _ASTNODEINFO_H
