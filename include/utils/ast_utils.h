#ifndef _AST_UTILS_H_
#define _AST_UTILS_H_

#include <clang-c/Index.h>
#include <string>
#include "ast_node_info.h"

// Get the source location (file, line, column) of a cursor
Location getCursorLocation(CXCursor cursor);

// Get the spelling (name) of a cursor
std::string getCursorSpelling(CXCursor cursor);

// Get the spelling (name) of a type
std::string getTypeSpelling(CXType type);

#endif
