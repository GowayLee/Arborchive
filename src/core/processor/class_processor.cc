#include "core/processor/class_processor.h"
#include "util/logger/macros.h"
#include <iostream>

void process(const clang::SourceLocation beginLoc,
             const clang::SourceLocation endLoc) {
  LOG_DEBUG << "Handling Class Declaration: " << std::endl;
}