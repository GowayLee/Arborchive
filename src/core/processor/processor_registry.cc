#include "core/processor/base_processor.h"

std::map<CXCursorKind, std::function<std::unique_ptr<BaseProcessor>()>>
    BaseProcessor::registry;

// Make sure to include the necessary headers
#include "core/processor/class_processor.h"

// Make LSP happy
extern ClassDeclProcessor dummyClassDeclProcessor;
