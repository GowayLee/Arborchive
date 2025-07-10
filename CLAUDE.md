# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Arborchive is a C++ static analysis tool that parses C/C++ source code using Clang's AST and stores structural information in a SQLite database. The name combines "Arbor" (tree, referencing AST) and "Archive" (storage).

## Build Commands

```bash
# Build the project (default)
make

# Build with debug flags
make debug

# Build with release optimizations  
make release

# Build and run with test file
make run

# Clean build artifacts
make clean

# Show help
make help
```

## Dependencies

- **clang** (version 19.1.7+) - for AST parsing
- **SQLite3** - for data storage
- **sqlite_orm** - header-only ORM library (included)
- **Python 3** - for code generation scripts

## Architecture

The project follows a layered, processor-based architecture:

1. **Router Layer** (`src/core/router.cc`) - Central coordinator managing compilation
2. **AST Visitor** (`src/core/ast_visitor.cc`) - Implements Clang's RecursiveASTVisitor 
3. **Processor Layer** (`src/core/processor/`) - Specialized handlers for different AST nodes:
   - `function_processor.cc` - Functions and calls
   - `variable_processor.cc` - Variables and references  
   - `type_professor.cc` - Type information
   - `stmt_processor.cc` - Statements (if, for, while, etc.)
   - `expr_processor.cc` - Expressions and operators
4. **Storage Layer** (`src/db/`) - SQLite database with caching and dependency management

## Key Components

- **StorageFacade** (`src/db/storage_facade.cc`) - Singleton providing unified database access
- **DependencyManager** (`src/db/dependency_manager.cc`) - Handles forward references and circular dependencies
- **Cache System** (`include/db/cache_repository.h`) - Type-safe in-memory caching
- **Key Generators** (`src/util/key_generator/`) - Generate unique identifiers for AST nodes

## Configuration

The project uses TOML configuration files. See `config.example.toml` for reference:

```bash
# Run with custom config
./build/demo -c config.example.toml -s tests/slight-case.cc -o tests/ast.db
```

## Code Generation

The build system automatically generates SQLite ORM instantiations:

```bash
# Regenerate instantiations (happens automatically during build)
python3 scripts/generate_instantiations.py
```

## Test Files

Test C++ files are located in `tests/`:
- `slight-case.cc` - Basic test cases
- `moderate-case.cc` - Intermediate complexity
- `intense-case.cc` - Complex scenarios

## Development Notes

- The project uses custom Makefile with separate compilation rules for LLVM-dependent and standard code
- Database schema is comprehensive with 300+ tables inspired by CodeQL's C++ database structure
- Two-phase processing: immediate AST processing + deferred dependency resolution
- Thread-safe ID generation and logging systems
- Supports modern C++ features including templates, coroutines, and C++20 constructs

## Claude Operational Guidelines

- Using `make` command to compile the project will create a large amount of context, which is really costy. So, never request `make` actively, unless the user tell you to do so.

## Implementation Guidelines

- When implementing new features or components, always follow existing code patterns:
  - Use the ASTVisitor framework in `@src/core/ast_visitor.cc`
  - Manage database records or entities under `@include/model/db/`
  - Implement core logics under `@src/core/processor/`