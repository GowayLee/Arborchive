#ifndef _BASE_PROCESSOR_H_
#define _BASE_PROCESSOR_H_

#include "db/async_manager.h"
#include "db/dependency_manager.h"
#include <clang/AST/ASTContext.h>

class BaseProcessor {
protected:
  DependencyManager &dep_manager_ = DependencyManager::getInstance();
  AsyncDatabaseManager &db_manager_ = AsyncDatabaseManager::getInstance();
  clang::ASTContext *ast_context_ = nullptr;

public:
  BaseProcessor(clang::ASTContext *ast_context) : ast_context_(ast_context) {};
  ~BaseProcessor() = default;
};

#endif // _BASE_PROCESSOR_H_
