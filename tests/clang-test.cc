#include <iostream>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

int main() {
  clang::Stmt* stmt = nullptr; // 可能需要包含更具体的 Stmt 子类
  std::cout << "Hello, Clang AST!" << std::endl;
  return 0;
}
