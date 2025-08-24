# AGENT_GUIDE.md

This guide provides comprehensive documentation for agents working with the Arborchive codebase. It covers the architecture, build system, key components, and development workflow.

## Project Overview

Arborchive is a sophisticated C++ static analysis tool that parses C/C++ source code using Clang's AST and stores structural information in a SQLite database. The name combines "Arbor" (tree, referencing AST) and "Archive" (storage).

### Key Capabilities

- **Modern C++ Support**: Full support for C++20 features including coroutines, templates, concepts, and more
- **Comprehensive Analysis**: 300+ database tables inspired by CodeQL's C++ database structure
- **Two-Phase Processing**: Immediate AST processing + deferred dependency resolution
- **Thread-Safe Architecture**: Concurrent processing with proper synchronization
- **Flexible Configuration**: TOML-based configuration system
- **Enhanced Logging**: Thread-safe logging with batch processing and multiple output targets

## Architecture Overview

The project follows a layered, processor-based architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────────┐
│                    CLI Interface Layer                       │
├─────────────────────────────────────────────────────────────┤
│                      Router Layer                           │
├─────────────────────────────────────────────────────────────┤
│                    AST Visitor Layer                        │
├─────────────────────────────────────────────────────────────┤
│                   Processor Layer                           │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │ Function    │ │ Variable    │ │ Type        │           │
│  │ Processor   │ │ Processor   │ │ Processor   │           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │ Class       │ │ Namespace   │ │ Stmt        │           │
│  │ Processor   │ │ Processor   │ │ Processor   │           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
│  ┌─────────────┐ ┌─────────────┐                         │
│  │ Expr        │ │ Location    │                         │
│  │ Processor   │ │ Processor   │                         │
│  └─────────────┘ └─────────────┘                         │
├─────────────────────────────────────────────────────────────┤
│                   Storage Layer                            │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │ Storage     │ │ Dependency  │ │ Cache       │           │
│  │ Facade      │ │ Manager     │ │ System      │           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
├─────────────────────────────────────────────────────────────┤
│                   Utility Layer                            │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │ IDGenerator │ │ Logger      │ │ Key         │           │
│  │ System      │ │ System      │ │ Generators  │           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
└─────────────────────────────────────────────────────────────┘
```

## Build System

### Build Commands

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

### Build Architecture

The Makefile implements a sophisticated build system that separates LLVM-dependent code from standard C++ code:

```makefile
# LLVM-dependent sources (require LLVM flags)
LLVM_SRCS = $(wildcard $(SRC_DIR)/core/*.cc \
                   $(SRC_DIR)/util/key_generator/*.cc \
                   $(SRC_DIR)/core/processor/*.cc)

# Standard C++ sources (standard compilation)
NORMAL_SRCS = $(wildcard $(SRC_DIR)/*.cc \
                     $(SRC_DIR)/interface/*.cc \
                     $(SRC_DIR)/util/*.cc \
                     $(SRC_DIR)/db/*.cc)
```

### Key Build Features

- **Separate Compilation**: LLVM-dependent and standard code compiled with different flags
- **Automatic Code Generation**: SQLite ORM instantiations generated during build
- **Dependency Tracking**: Automatic dependency file generation
- **Multiple Build Targets**: Debug, release, and run configurations

## Core Components

### 1. Router Layer (`src/core/router.cc`)

The Router serves as the central coordinator for the entire compilation process:

```cpp
class Router {
public:
  static Router &getInstance();
  void processCompilation(const Configuration &config);

private:
  void parseAST(const std::string &source_path);
};
```

**Key Responsibilities:**

- Initialize and coordinate all system components
- Manage the compilation pipeline
- Handle configuration and setup
- Coordinate AST parsing and processing

### 2. AST Visitor (`src/core/ast_visitor.cc`)

Implements Clang's RecursiveASTVisitor to traverse the AST:

```cpp
class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
private:
  // Processors for different AST node types
  std::unique_ptr<ClassDeclProcessor> class_decl_processor_;
  std::unique_ptr<FunctionProcessor> function_processor_;
  std::unique_ptr<NamespaceProcessor> namespace_processor_;
  std::unique_ptr<VariableProcessor> variable_processor_;
  std::unique_ptr<TypeProcessor> type_processor_;
  std::unique_ptr<StmtProcessor> stmt_processor_;
  std::unique_ptr<ExprProcessor> expr_processor_;

public:
  // Visit methods for different AST node types
  bool VisitFunctionDecl(clang::FunctionDecl *decl);
  bool VisitVarDecl(clang::VarDecl *decl);
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl);
  bool VisitNamespaceDecl(clang::NamespaceDecl *decl);
  // ... more visit methods
};
```

**Supported AST Node Types:**

- **Declarations**: Functions, variables, types, classes, namespaces
- **Statements**: If, for, while, switch, compound statements
- **Expressions**: Calls, operators, literals, references
- **C++ Features**: Templates, coroutines, exceptions, inheritance

### 3. Processor Layer (`src/core/processor/`)

Specialized handlers for different AST node types:

#### Base Processor (`base_processor.h`)

```cpp
class BaseProcessor {
protected:
  clang::ASTContext *ast_context_;

public:
  BaseProcessor(clang::ASTContext *ast_context);
};
```

#### Specialized Processors

**Function Processor** (`function_processor.cc`):

- Handles function declarations, definitions, and calls
- Supports C++ methods, constructors, destructors, conversion operators
- Manages function signatures, parameters, and return types

**Variable Processor** (`variable_processor.cc`):

- Processes variable declarations and references
- Handles local variables, global variables, and parameters
- Manages variable scoping and lifetime

**Type Processor** (`type_professor.cc`):

- Processes type declarations and definitions
- Handles built-in types, user-defined types, and template types
- Manages type relationships and inheritance

**Class Processor** (`class_processor.cc`):

- Processes class and struct declarations
- Handles inheritance, access specifiers, and member variables
- Manages class templates and specializations

**Namespace Processor** (`namespace_processor.cc`):

- Processes namespace declarations and aliases
- Handles nested namespaces and inline namespaces
- Manages namespace scope and member resolution

**Statement Processor** (`stmt_processor.cc`):

- Processes control flow statements
- Handles if, for, while, switch statements
- Manages statement blocks and scoping

**Expression Processor** (`expr_processor.cc`):

- Processes expressions and operators
- Handles function calls, operators, literals
- Manages expression evaluation and type resolution

**Location Processor** (`location_processor.cc`):

- Tracks source code locations
- Manages file, line, and column information
- Handles location resolution and mapping

### 4. Storage Layer (`src/db/`)

#### Storage Facade (`storage_facade.cc`)

Singleton providing unified database access:

```cpp
class StorageFacade {
public:
  static StorageFacade &getInstance();
  void initOrm(const DatabaseConfig &config);

  // Database operations
  template<typename T> void insert(const T &entity);
  template<typename T> void update(const T &entity);
  template<typename T> std::vector<T> getAll();
};
```

#### Dependency Manager (`dependency_manager.cc`)

Handles forward references and circular dependencies:

```cpp
class DependencyManager {
public:
  void addDependency(const std::string &from, const std::string &to);
  void resolveDependencies();
  bool hasCircularDependency();
};
```

#### Cache System (`cache_repository.h`)

Type-safe in-memory caching:

```cpp
template<typename T>
class CacheRepository {
public:
  void insert(const KeyType &key, int id);
  std::optional<int> find(const KeyType &key);
  void clear();
};
```

### 5. Utility Systems

#### ID Generator (`id_generator.cc`)

Thread-safe global unique ID generation:

```cpp
class IDGenerator {
private:
  static std::mutex mutex_;
  static std::atomic<int> global_id_;

public:
  template<typename T> static int generateId();
  template<typename T> static int getLastGeneratedId();
  template<typename T> static void generateAndSetId(const T &model);
};
```

**Usage:**

```cpp
// Generate ID for any model with 'id' member
int function_id = GENID(Function);
int variable_id = GENID(Variable);

// Generate and set ID directly
GENID(Function);
function_model.id = IDGenerator::getLastGeneratedId<Function>();
```

#### Logger System (`logger/`)

Advanced thread-safe logging system:

```cpp
class Logger {
private:
  ThreadSafeQueue<LogMessage> queue_;
  std::atomic<bool> running_;
  std::thread worker_thread_;
  std::vector<LogMessage> batch_buffer_;

public:
  static Logger &getInstance();
  void init();
  bool loadConfig(const LoggerConfig &config);
  void stop();

  // Logging interface
  Logger &operator()(LogLevel level, const char *file, int line);
  template<typename T> Logger &operator<<(const T &value);
};
```

**Features:**

- Thread-safe operation with dedicated worker thread
- Batch processing for improved performance
- Multiple output targets (console, file)
- Configurable log levels and formatting
- Performance timing capabilities

**Usage:**

```cpp
// Simple logging
LOG_INFO << "Processing function: " << function_name;
LOG_ERROR << "Failed to parse AST: " << error_message;
LOG_DEBUG << "Variable declared at line: " << line_number;

// Performance logging
LOG_PERF << "AST parsing completed in " << duration << "ms";
```

#### Key Generators (`key_generator/`)

Specialized key generation for different entity types:

```cpp
namespace KeyGen {
namespace Values {
  KeyType makeKey(const std::string &value);
}
namespace ValueText {
  KeyType makeKey(const std::string &text);
}
namespace Function {
  KeyType makeKey(const clang::FunctionDecl *decl);
}
// ... more key generators
}
```

## Configuration System

### TOML Configuration Structure

The project uses TOML configuration files for flexible configuration:

```toml
[general]
source_path = "tests/slight-case.cc"
output_path = "tests/ast.db"

[compilation]
include_paths = [
    "/usr/local/include",
    "third_party/boost_1_83",
]
defines = [
    "DEBUG_MODE=1",
    "USE_FAST_ALGO",
]
cxx_standard = "c++20"
flags = [
    "-fexceptions",
    "-fcoroutines",
    "-Wno-deprecated",
    "-O0"
]

[database]
path = "tests/ast.db"
batch_size = 10
cache_size_mb = 64
journal_mode = "WAL"
synchronous = "NORMAL"

[logging]
level = "DEBUG"
file = ""
is_to_console = true
batch_size = 5
enable_perf_logging = false
```

### Configuration Loading

```cpp
// Load from TOML file
auto &configLoader = ConfigLoader::getInstance();
if (!configLoader.loadFromFile("config.toml")) {
  // Handle error
}

// Merge command line arguments
configLoader.mergeFromCli(clargs);

// Access configuration
const auto &config = configLoader.getConfig();
const auto &db_config = config.database;
const auto &logger_config = config.logger;
```

## Database Schema

### Overview

The database schema consists of 300+ tables inspired by CodeQL's C++ database structure:

#### Core Tables

- **`compilation`**: Compilation unit information
- **`element`**: Generic element storage
- **`location`**: Source code location tracking
- **`function`**: Function declarations and definitions
- **`variable`**: Variable declarations and references
- **`type`**: Type information and relationships
- **`stmt`**: Statement information and control flow
- **`expr`**: Expression information and operators

#### Specialized Tables

- **`class`**: Class and struct information
- **`namespace`**: Namespace declarations and aliases
- **`container`**: Container types and relationships
- **`declaration`**: Generic declaration information
- **`parameterized_element`**: Template and generic types

#### Supporting Tables

- **`values`**: Literal values
- **`value_text`**: Text content storage
- \*\*Location dependencies and relationships
- \*\*Type inheritance and specialization
- \*\*Function call relationships

### Schema Features

- **Comprehensive Coverage**: Handles all C++ language features
- **Relationship Tracking**: Maintains relationships between different entities
- **Location Mapping**: Precise source code location tracking
- **Type Safety**: Strong typing with proper relationships
- **Extensibility**: Designed for easy extension and modification

### Code Generation

The build system automatically generates SQLite ORM instantiations:

```bash
# Manual regeneration (happens automatically during build)
python3 scripts/generate_instantiations.py
```

This generates `src/db/storage_facade_instantiations.inc` with all necessary ORM mappings.

## Development Workflow

### Getting Started

1. **Clone and Setup**

   ```bash
   git clone <repository-url>
   cd Arborchive
   ```

2. **Build the Project**

   ```bash
   make
   ```

3. **Run Tests**

   ```bash
   make run
   ```

4. **Custom Configuration**
   ```bash
   ./build/demo -c config.example.toml -s your_source.cc -o output.db
   ```

### Development Process

1. **Understand the Architecture**: Review the architecture overview and component descriptions
2. **Familiarize with Code Structure**: Explore the `src/` and `include/` directories
3. **Set Up Development Environment**: Ensure LLVM/Clang and SQLite3 are installed
4. **Choose Your Focus**: Select a component to work on based on your interests
5. **Follow Coding Standards**: Adhere to existing patterns and conventions
6. **Test Your Changes**: Use the provided test files and add new tests as needed
7. **Document Your Work**: Update relevant documentation and comments

### Code Structure

```
Arborchive/
├── src/                    # Source code
│   ├── core/              # Core components
│   │   ├── processor/     # AST node processors
│   │   ├── ast_visitor.cc # AST traversal
│   │   └── router.cc      # Central coordinator
│   ├── db/                # Database operations
│   ├── interface/         # CLI and configuration
│   ├── util/              # Utility functions
│   └── main.cc           # Entry point
├── include/               # Header files
│   ├── core/             # Core component headers
│   ├── db/               # Database headers
│   ├── model/            # Data models
│   └── util/             # Utility headers
├── tests/                 # Test files
├── scripts/              # Build and utility scripts
└── docs/                 # Documentation
```

## Testing Strategy

### Test Files

#### Unit Tests (`tests/unit-tests/`)

- **Function Tests**: Coroutine handling, templates, exceptions, typedefs
- **Namespace Tests**: Namespace declarations and member resolution
- **Type Tests**: Type relationships and inheritance
- **Expression Tests**: Operator precedence and evaluation

#### Integration Tests (`tests/`)

- **`slight-case.cc`**: Basic functionality and simple constructs
- **`moderate-case.cc`**: Intermediate complexity scenarios
- **`intense-case.cc`**: Complex language features and edge cases
- **`temp.cc`**: Temporary testing and debugging

### Running Tests

```bash
# Build and run basic test
make run

# Build and run with custom test file
make run
./build/demo -c config.example.toml -s tests/your-test.cc -o tests/output.db

# Run specific test scenarios
./build/demo -c config.example.toml -s tests/unit-tests/function/coroutine.cc -o tests/coroutine.db
```

### Test Development

When adding new features:

1. **Create Test Cases**: Add appropriate test files in `tests/` or `tests/unit-tests/`
2. **Verify Functionality**: Test both normal and edge cases
3. **Check Database Output**: Verify that the database contains expected data
4. **Performance Testing**: Ensure reasonable performance for large inputs
5. **Memory Testing**: Check for memory leaks and proper cleanup

## Implementation Guidelines

### Code Patterns

#### 1. Singleton Pattern

```cpp
class MyClass {
public:
  static MyClass &getInstance() {
    static MyClass instance;
    return instance;
  }

private:
  MyClass() = default;
  MyClass(const MyClass &) = delete;
  MyClass &operator=(const MyClass &) = delete;
};
```

#### 2. Template-Based ID Generation

```cpp
// Use the IDGenerator for all database entities
int function_id = GENID(Function);
DbModel::Function function;
function.id = function_id;
// ... populate other fields
STG.insert(function);
```

#### 3. Logging Patterns

```cpp
// Always use appropriate log levels
LOG_DEBUG << "Detailed debugging information";
LOG_INFO << "General information about execution";
LOG_WARN << "Warning conditions that should be noted";
LOG_ERROR << "Error conditions that need attention";

// Use performance logging for timing
auto start = std::chrono::high_resolution_clock::now();
// ... operation ...
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
LOG_PERF << "Operation completed in " << duration.count() << "ms";
```

#### 4. Error Handling

```cpp
// Use try-catch blocks for critical operations
try {
  // Operation that might throw
} catch (const std::exception &e) {
  LOG_ERROR << "Operation failed: " << e.what();
  return false;
}
```

### Database Operations

#### 1. Insert Operations

```cpp
// Generate ID and insert
GENID(Function);
DbModel::Function function;
function.id = IDGenerator::getLastGeneratedId<Function>();
function.name = "myFunction";
STG.insert(function);
```

#### 2. Cache Usage

```cpp
// Check cache first
auto cached_id = SEARCH_FUNCTION_CACHE(function_key);
if (cached_id) {
  return *cached_id;
}

// Not in cache, process and insert
int function_id = GENID(Function);
// ... process function ...
INSERT_FUNCTION_CACHE(function_key, function_id);
return function_id;
```

### Adding New Processors

When adding new AST node processors:

1. **Create Processor Class**: Inherit from `BaseProcessor`
2. **Implement Visit Method**: Add corresponding `Visit` method in `ASTVisitor`
3. **Register Processor**: Initialize processor in `ASTVisitor::initProcessors()`
4. **Add Database Models**: Create corresponding database models in `include/model/db/`
5. **Update Schema**: Add table definitions in `include/db/table_defs/`
6. **Generate ORM**: Run code generation script if needed
7. **Add Tests**: Create appropriate test cases

### Refactoring Guidelines

- **Don't Remove Code**: Comment out code instead of deleting for change tracking
- **Update Documentation**: Keep documentation synchronized with code changes
- **Test Thoroughly**: Ensure refactoring doesn't break existing functionality
- **Follow Patterns**: Maintain consistency with existing code patterns
- **Performance Considerations**: Monitor performance impact of changes

## Key Files Reference

### Core Components

- **`src/main.cc`**: Program entry point and initialization
- **`src/core/router.cc`**: Central coordination and compilation management
- **`src/core/ast_visitor.cc`**: AST traversal and node processing
- **`src/core/processor/`**: Specialized AST node processors
- **`src/db/storage_facade.cc`**: Database operations and ORM management
- **`src/db/dependency_manager.cc`**: Dependency resolution and circular dependency handling

### Utilities

- **`src/util/id_generator.cc`**: Global unique ID generation
- **`src/util/logger/`**: Thread-safe logging system
- **`src/util/key_generator/`**: Entity key generation utilities
- **`src/util/hires_timer.cc`**: High-resolution timing utilities

### Interface

- **`src/interface/cli.cc`**: Command-line interface and argument parsing
- **`src/interface/config_loader.cc`**: Configuration file loading and management

### Models and Definitions

- **`include/model/db/`**: Database entity models
- **`include/db/table_defs/`**: Database table definitions
- **`include/model/config/`**: Configuration data models
- **`include/core/processor/`**: Processor class definitions

### Build and Scripts

- **`Makefile`**: Build system and compilation rules
- **`scripts/generate_instantiations.py`**: SQLite ORM code generation
- **`config.example.toml`**: Example configuration file

## Troubleshooting

### Common Issues

1. **Build Failures**
   - Ensure LLVM/Clang development packages are installed
   - Check that all dependencies are available
   - Verify LLVM version compatibility (19.1.7+)

2. **Database Connection Issues**
   - Check database file permissions
   - Verify SQLite3 installation
   - Review configuration settings

3. **AST Parsing Errors**
   - Verify source file syntax
   - Check compilation flags and includes
   - Review C++ standard compatibility

4. **Memory Issues**
   - Check for proper cleanup in destructors
   - Verify smart pointer usage
   - Monitor memory usage with large inputs

### Debug Mode

Use debug build for detailed logging and debugging:

```bash
make debug
./build/demo -c config.example.toml -s tests/slight-case.cc -o tests/ast.db
```

### Performance Analysis

Enable performance logging to analyze bottlenecks:

```toml
[logging]
enable_perf_logging = true
level = "INFO"
```

## Contributing

### Code Style

- Follow existing code formatting and style
- Use meaningful variable and function names
- Add appropriate comments for complex logic
- Maintain consistent indentation and spacing

### Testing Requirements

- All changes must pass existing tests
- Add new tests for new functionality
- Test both normal and edge cases
- Verify performance impact

### Documentation Updates

- Update relevant documentation sections
- Add comments for new functions and classes
- Update configuration examples if needed
- Document new features and capabilities

### Code Review

- Ensure code follows established patterns
- Verify proper error handling
- Check for potential memory leaks
- Review performance implications

---

This guide provides a comprehensive overview of the Arborchive codebase. For specific questions or detailed implementation details, refer to the source code documentation and comments throughout the codebase.

