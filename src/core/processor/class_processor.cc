#include "core/processor/class_processor.h"
#include "util/logger/macros.h"
#include <clang-c/Index.h>
#include <iostream>

void ClassDeclProcessor::handle(CXCursor cursor) {
    std::string functionName =
        clang_getCString(clang_getCursorSpelling(cursor));
    LOG_DEBUG << "Handling Class Declaration: " << functionName << std::endl;
}