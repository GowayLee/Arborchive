#ifndef _KEY_GENERATOR_TYPE_H_
#define _KEY_GENERATOR_TYPE_H_

#include "db/cache_repository.h"
#include "model/db/type.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Type.h>
#include <string>

#define SEARCH_TYPE_CACHE(type)                                                \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::Type>>()                         \
      .find(type)

#define SEARCH_USERTYPE_CACHE(type)                                            \
  CacheManager::instance()                                                     \
      .getRepository<CacheRepository<DbModel::UserType>>()                     \
      .find(type)

using KeyType = std::string;
using namespace clang;

std::string getScopePath(const NamedDecl *decl);
std::string getDeclName(const NamedDecl *decl);
std::string getTemplateArgs(const NamedDecl *decl);
std::string getTemplateParamListAsString(const TemplateParameterList *params);
std::string getTemplateArgumentListAsString(const TemplateArgumentList &args);

namespace KeyGen {

namespace Type {

// For clang::QualType
KeyType makeKey(const QualType &qualType, const ASTContext &ctx);

// For clang::NamedDecl
KeyType makeKey(const NamedDecl *decl, const ASTContext &ctx);

// clang::TemplateDecl -> clang::NamedDecl
KeyType makeKey(const TemplateDecl *decl, const ASTContext &ctx);

// clang::TypeDecl -> clang::NamedDecl
KeyType makeKey(const TypeDecl *decl, const ASTContext &ctx);

} // namespace Type

} // namespace KeyGen

#endif // _KEY_GENERATOR_TYPE_H_
