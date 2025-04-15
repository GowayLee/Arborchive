#ifndef _ROUTER_H_
#define _ROUTER_H_

#include "model/config/configuration.h"
#include <clang/AST/ASTContext.h>
#include <string>

class Router {
public:
  Router(const Router &other) = delete;
  Router &operator=(const Router &) = delete;

  static Router &getInstance() {
    static Router instance;
    return instance;
  }

  void processCompilation(const Configuration &config);

private:
  ~Router() = default;
  Router() = default;

  void parseAST(const std::string &source_path);
};

#endif // _ROUTER_H_
