#include "util/key_generator/element.h"
#include "util/key_generator/function.h"

namespace KeyGen {

namespace Element {

KeyType makeKeyFromFuncKey(const KeyType funcKey) {
  return "function-" + funcKey;
}

KeyType makeKey(const FunctionDecl *FD, ASTContext *Context) {
  return makeKeyFromFuncKey(KeyGen::Function::makeKey(FD, Context));
}

// KeyType makeKeyFromStmtBlock(KeyType funcKey) { return "stmtblock-" +
// funcKey; } KeyType makeKeyFromRequireExpr(KeyType funcKey) { return
// "requireexpr-" + funcKey; };

} // namespace Element
} // namespace KeyGen
