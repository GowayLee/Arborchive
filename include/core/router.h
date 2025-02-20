#ifndef _ROUTER_H_
#define _ROUTER_H_

#include "core/processor/base_processor.h"
#include "interface/clang_indexer.h"
#include <clang-c/Index.h>
#include <string>

class Router {
public:
  Router(const Router &other) = delete;
  Router &operator=(const Router &) = delete;

  static Router &getInstance() {
    static Router instance;
    return instance;
  }
  void parseAST(const std::string &filename);

private:
  ~Router() = default;
  Router() = default;

  static CXChildVisitResult visitCursor(CXCursor cursor, CXCursor parent,
                                        CXClientData client_data);
};

#endif // _ROUTER_H_
