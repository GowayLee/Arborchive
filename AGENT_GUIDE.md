# AGENT_GUIDE.md

Guide for agents working with the Arborchive codebase - a C++ static analysis tool that parses C/C++ source code using Clang's AST and stores structural information in SQLite.

## Project Overview

**Arborchive** (from Latin "Arbor" + "Archive") symbolizes parsing and storing code structure as a tree (AST). It uses Clang to parse C/C++ code and stores AST node information in SQLite.

## Architecture

Layered, processor-based architecture:

```
CLI → ConfigLoader → Router → ClangASTManager → ASTVisitor → Processors → Storage
```

**Core Layers:**

- **Router** (`src/core/router.cc`): Central coordinator, manages compilation pipeline
- **ClangASTManager** (`src/core/clang_ast_manager.cc`): Manages Clang AST loading and processing
- **CompilationRecorder** (`src/core/compilation_recorder.cc`): Records compilation metadata and timing
- **AST Visitor** (`src/core/ast_visitor.cc`): Traverses Clang AST, delegates to processors
- **Processors** (`src/core/processor/`): Specialized handlers for AST node types
- **Storage** (`src/db/`): Database operations, dependency resolution
- **Utilities** (`src/util/`): ID generation, logging, key generation

## Implemented Components

**Processors (7/7):**
| Processor | File | Responsibilities |
|-----------|------|------------------|
| FunctionProcessor | `function_processor.cc` | declarations, definitions, calls, constructors, destructors, conversion, deduction guides |
| VariableProcessor | `variable_processor.cc` | local, global, member, parameters, fields |
| TypeProcessor | `type_processor.cc` | built-in, user-defined, templates, derived types, enums, typedefs |
| StmtProcessor | `stmt_processor.cc` | control flow (if, for, while, switch, do, return, compound, decl) |
| ExprProcessor | `expr_processor.cc` | expressions, operators, literals, decl references, calls |
| NamespaceProcessor | `namespace_processor.cc` | namespaces, aliases |
| SpecifierProcessor | `specifier_processor.cc` | storage classes, type qualifiers, function specifiers |

**Helper Classes:**

- `DerivedTypeHelper` - Derived type processing utilities
- `CoroutineHelper` - C++20 coroutine support
- `ExprHelper` - Expression processing utilities
- `UserTypeHelper` - User-defined type helpers
- `LocationProcessor` - Source location tracking

**Database Models (12):**

- `class.h` - Class/struct definitions
- `compilation.h` - Compilation metadata
- `container.h` - Container relationships
- `declaration.h` - Declaration metadata
- `element.h` - Base element model
- `expr.h` - Expression models
- `function.h` - Function models
- `location.h` - Source location models
- `specifiers.h` - Type/function specifiers
- `stmt.h` - Statement models
- `type.h` - Type models
- `variable.h` - Variable models

**Utility Systems:**

- Thread-safe ID generation (`GENID` macro via `IDGenerator`)
- Advanced logging (`LOG_INFO`, `LOG_ERROR`, `LOG_DEBUG`, `LOG_PERF`)
- Key generators (7 modules): element, expr, function, stmt, type, values, variable
- Dependency resolution and circular dependency handling
- High-resolution timing (`HighResTimer`)

**Interface Components:**

- `CLI` (`src/interface/cli.cc`) - Command-line interface
- `ConfigLoader` (`src/interface/config_loader.cc`) - TOML configuration loading

## Database Schema

300+ tables inspired by CodeQL. Core tables:

- `compilation`, `element`, `location`
- `function`, `variable`, `type`, `stmt`, `expr`
- `class`, `namespace`, `container`, `specifiers`

**Cache System:** Template-based repositories in `include/db/cache_repository.h`

- `CacheRepository<Model, KeyType, IdType>` - Generic cache template
- `CacheManager` singleton - Manages all cache repositories

**Layered Design** (from `docs/layers.md`):

1. Core Essentials: files, folders, locations, compilation info
2. Basic Syntax: expressions, statements, types, variables, functions
3. Type System: built-in, derived, user-defined, qualifiers
4. Functions & Scopes: definitions, parameters, calls, overloading
5. Classes & Inheritance: members, inheritance, virtual functions
6. Templates: definitions, instantiation, specialization
7. Macros: definitions, expansion
8. Control Flow: conditionals, loops, jumps, exceptions
9. Compilation & Linking: compilation units, external data
10. Preprocessing: directives, includes

## Code Patterns

### ID Generation

```cpp
int function_id = GENID(Function);
DbModel::Function function;
function.id = function_id;
STG.insert(function);
```

### Logging

```cpp
LOG_INFO << "Processing: " << name;
LOG_ERROR << "Failed: " << error;
LOG_DEBUG << "Details: " << details;
LOG_PERF << "Operation took " << duration << "ms";
```

### Database Operations with Cache

```cpp
// Get cache repository
auto& cache = CacheManager::instance().getRepository<FunctionCacheRepository>();

// Check cache first
auto cached_id = cache.find(key);
if (cached_id) return *cached_id;

// Process and insert
GENID(Function);
DbModel::Function function;
function.id = IDGenerator::getLastGeneratedId<Function>();
// ... populate fields ...
STG.insert(function);
cache.insert(key, function.id);
```

### Router Processing Pattern

```cpp
void Router::processCompilation(const Configuration &config) {
  // Create compilation record
  CompRecorder &recorder = CompRecorder::getInstance();
  recorder.createCompilation(config.compilation.working_directory);

  // Load config and process AST
  ClangASTManager::getInstance().loadConfig(config);
  parseAST(config.general.source_path);

  // Resolve dependencies
  DependencyManager::instance().resolveDependencies();
}
```

## Key Files

**Core:**
| File | Purpose |
|------|---------|
| `src/main.cc` | Entry point |
| `src/core/router.cc` | Central coordinator |
| `src/core/clang_ast_manager.cc` | Clang AST management |
| `src/core/compilation_recorder.cc` | Compilation metadata |
| `src/core/ast_visitor.cc` | AST traversal |
| `src/core/processor/*.cc` | 7 specialized processors |
| `src/core/srcloc_recorder.cc` | Location tracking |

**Storage:**
| File | Purpose |
|------|---------|
| `src/db/storage_facade.cc` | Database operations (`STG` singleton) |
| `src/db/dependency_manager.cc` | Dependency resolution |

**Interface:**
| File | Purpose |
|------|---------|
| `src/interface/cli.cc` | CLI implementation |
| `src/interface/config_loader.cc` | TOML config loading |

**Utilities:**
| File | Purpose |
|------|---------|
| `src/util/id_generator.cc` | Thread-safe IDs |
| `src/util/logger.cc` | Logging system |
| `src/util/key_generator/*.cc` | 7 key generation modules |
| `include/util/hires_timer.h` | High-resolution timing |

**Models:**
| Directory | Contents |
|-----------|----------|
| `include/model/db/*.h` | 12 database models |
| `include/db/cache_repository.h` | Cache templates |
| `include/core/processor/*.h` | Processor definitions & helpers |

**Scripts:**
| File | Purpose |
|------|---------|
| `scripts/generate_instantiations.py` | Generate ORM instantiations |
| `scripts/convert2dl.sh` | Convert SQLite to Datalog |

## Implementation Status

**Completion: ~85-90%**

✅ **Fully Implemented:**

- All 7 processors with comprehensive C++20 support
- 12 database models integrated with ORM
- Thread-safe utilities (logging, ID generation, key generators)
- Dependency resolution for forward references
- Modern C++ features: coroutines, concepts, templates, inheritance
- ClangASTManager for AST processing
- CompilationRecorder for metadata tracking
- TOML configuration support

⚠️ **Partially Implemented (TODO):**

- Friend declarations (`ast_visitor.cc:~240`)
- Template declarations (`ast_visitor.cc:~243`)
- ImplicitCastExpr DerivedType recording (`ast_visitor.cc:~120`)

## Makefile Commands

```bash
make debug -j 8    # Build with debug flags
make clean    # Clean build artifacts
make help     # Show all commands
```

## Adding New Processors

1. Create processor class inheriting from `BaseProcessor` in `src/core/processor/`
2. Add `Visit` method in `ASTVisitor` for the AST node type
3. Initialize processor in `ASTVisitor::initProcessors()`
4. Create database model in `include/model/db/`
5. Add table definition in `include/db/table_defs/`
6. Run `python3 scripts/generate_instantiations.py` for ORM
7. Create key generator in `src/util/key_generator/` if needed
8. Create helper classes in `include/core/processor/` if needed

## Development Guidelines

- **Don't delete code** - comment out for change tracking
- **Follow existing patterns** - maintain consistency
- **Use caches** - always check cache before processing
- **Log appropriately** - use correct log levels (INFO for progress, ERROR for failures, DEBUG for details, PERF for timing)
- **Test thoroughly** - verify no regressions
- **Monitor performance** - track impact of changes with `HighResTimer`

## Testing

Test files available in `tests/`:

- `slight-case.cc` - Minimal test case
- `moderate-case.cc` - Moderate complexity
- `intense-case.cc` - Complex C++20 features

Run with: `./build/demo -c ./config.example.toml -s ./tests/slight-case.cc -o ./tests/ast.db`

## Code Structure

```
src/
├── core/
│   ├── processor/         # 7 specialized processors
│   ├── ast_visitor.cc     # AST traversal
│   ├── clang_ast_manager.cc
│   ├── compilation_recorder.cc
│   ├── srcloc_recorder.cc
│   └── router.cc          # Coordinator
├── db/
│   ├── storage_facade.cc
│   ├── storage_facade_instantiations.inc
│   └── dependency_manager.cc
├── interface/
│   ├── cli.cc
│   └── config_loader.cc
├── util/
│   ├── id_generator.cc
│   ├── logger.cc
│   └── key_generator/     # 7 modules (element, expr, function, stmt, type, values, variable)
└── main.cc

include/
├── core/
│   ├── processor/         # Processor headers + helpers
│   ├── ast_visitor.h
│   ├── clang_ast_manager.h
│   ├── compilation_recorder.h
│   ├── router.h
│   └── srcloc_recorder.h
├── db/
│   ├── cache_repository.h # Cache templates
│   ├── dependency_manager.h
│   ├── storage.h
│   ├── storage_facade.h
│   └── table_defs/
├── model/db/              # 12 models
├── interface/
│   ├── cli.h
│   └── config_loader.h
└── util/
    ├── hires_timer.h
    ├── id_generator.h
    ├── key_generator/
    └── logger/

docs/
├── layers.md              # Database schema layers
├── datatable-list.txt     # Table definitions
├── scheme_info.md         # Schema reference
└── semmlecode.cpp.dbscheme # CodeQL scheme reference

scripts/
├── generate_instantiations.py
├── convert2dl.sh
└── scheme_tools/
```

## Dependencies

- **clang** 19.1.7 - AST parsing
- **sqlite_orm** (in `third_party/`) - ORM for SQLite
- **TOML** - Configuration parsing
