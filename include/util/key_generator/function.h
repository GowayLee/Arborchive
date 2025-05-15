#ifndef _KEY_GENERATOR_FUNCTION_H_
#define _KEY_GENERATOR_FUNCTION_H_

#include "db/cache_repository.h"
#include "util/logger/macros.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h> // For CXXMethodDecl etc.
#include <clang/AST/Mangle.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

#define SEARCH_FUNCTION_CACHE(type)                                            \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Function>>()                     \
      .find(type)

#define INSERT_FUNCTION_CACHE(key, id)                                         \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Function>>()                     \
      .insert(key, id)

using KeyType = std::string;
using namespace clang;

namespace KeyGen {

namespace Function {

static inline KeyType makeKey(const FunctionDecl *FD, ASTContext &Context) {
  if (!FD) {
    LOG_ERROR << "Null FunctionDecl" << std::endl;
    return "ERRORKEY";
  }

  const FunctionDecl *CanonicalFD = FD->getCanonicalDecl();
  if (!CanonicalFD) {
    LOG_ERROR << "Cannot get CanonicalDecl for FunctionDecl" << std::endl;
    return "ERRORKEY";
  }

  MangleContext *MangleCtx = Context.createMangleContext();
  if (!MangleCtx) {
    LOG_ERROR << "MangleCtx is not available for "
              << CanonicalFD->getQualifiedNameAsString() << std::endl;
    return "ERRORKEY";
  }

  std::string MangledName;
  llvm::raw_string_ostream Ostream(MangledName);

  if (const auto *CD = llvm::dyn_cast<clang::CXXConstructorDecl>(CanonicalFD)) {
    // For constructors
    clang::GlobalDecl GD(CD, clang::Ctor_Complete);
    MangleCtx->mangleName(GD, Ostream);
  } else if (const auto *DD =
                 llvm::dyn_cast<clang::CXXDestructorDecl>(CanonicalFD)) {
    // For destructors
    clang::GlobalDecl GD(DD, clang::Dtor_Complete);
    MangleCtx->mangleName(GD, Ostream);
  } else if (CanonicalFD->hasAttr<clang::CUDAGlobalAttr>()) {
    // Handle CUDA kernels
    clang::GlobalDecl GD(CanonicalFD, /*MultiVersionIndex=*/0);
    MangleCtx->mangleName(GD, Ostream);
  } else
    MangleCtx->mangleName(CanonicalFD, Ostream);

  Ostream.flush();
  if (MangledName.empty()) {
    LOG_ERROR << "Warning: Mangled name was empty for: "
              << CanonicalFD->getQualifiedNameAsString() << std::endl;
    return "__EMPTY_MANGLED__" + CanonicalFD->getQualifiedNameAsString();
  }
  return MangledName;
}

} // namespace Function

} // namespace KeyGen

#endif // _KEY_GENERATOR_FUNCTION_H_
