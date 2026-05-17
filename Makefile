# ==============================================
# Compiler Configuration
# ==============================================
CXXFLAGS ?= -Wall -Wextra -pedantic -Iinclude -g
PY = python3

# ==============================================
# LLVM Configuration
# ==============================================
# Prefer LLVM 19, but allow users to pass LLVM_CONFIG=/path/to/llvm-config-19.
LLVM_CONFIG ?= $(shell command -v llvm-config-19 2>/dev/null || command -v llvm-config19 2>/dev/null || command -v llvm-config 2>/dev/null)
LLVM_VERSION := $(shell $(LLVM_CONFIG) --version 2>/dev/null)
LLVM_MAJOR := $(shell echo "$(LLVM_VERSION)" | cut -d. -f1)

CHECK_TOOLCHAIN := $(if $(MAKECMDGOALS),$(filter-out clean help,$(MAKECMDGOALS)),all)
ifneq ($(CHECK_TOOLCHAIN),)
ifeq ($(strip $(LLVM_CONFIG)),)
$(error Arborchive requires LLVM 19. Please install LLVM 19 or run: make LLVM_CONFIG=/path/to/llvm-config-19)
endif
ifneq ($(LLVM_MAJOR),19)
$(error Arborchive requires LLVM 19, but LLVM_CONFIG=$(LLVM_CONFIG) reports "$(LLVM_VERSION)". Please run: make LLVM_CONFIG=/path/to/llvm-config-19)
endif
endif

LLVM_BINDIR := $(shell $(LLVM_CONFIG) --bindir 2>/dev/null)
LLVM_CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags 2>/dev/null)
LLVM_LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags 2>/dev/null)
LLVM_LIBDIR := $(shell $(LLVM_CONFIG) --libdir 2>/dev/null)
LLVM_LIBS := $(shell $(LLVM_CONFIG) --libs core 2>/dev/null)

# GNU make predefines CXX as g++; replace only that implicit default.
ifeq ($(origin CXX), default)
CXX := $(LLVM_BINDIR)/clang++
endif
CXX_EXISTS := $(shell command -v "$(firstword $(CXX))" >/dev/null 2>&1 && echo yes)
CXX_VERSION := $(shell $(CXX) --version 2>/dev/null | head -n 1)
ifneq ($(CHECK_TOOLCHAIN),)
ifeq ($(CXX_EXISTS),)
$(error Could not find clang++ next to llvm-config. Please run make CXX=/path/to/clang++)
endif
ifneq ($(findstring Apple clang,$(CXX_VERSION)),)
$(error Refusing to build Arborchive with Apple Clang while linking LLVM 19. Please run make CXX=$(LLVM_BINDIR)/clang++ or set CXX to an LLVM 19 clang++)
endif
endif

# ==============================================
# SQLite Discovery
# ==============================================
SQLITE_CFLAGS ?= $(shell pkg-config --cflags sqlite3 2>/dev/null)
SQLITE_LIBS ?= $(shell pkg-config --libs sqlite3 2>/dev/null || echo -lsqlite3)
TOML_CFLAGS ?=

# ==============================================
# Compiler and Linker Flags
# ==============================================
COMMON_CXXFLAGS = $(CXXFLAGS) $(LLVM_CXXFLAGS) $(SQLITE_CFLAGS) $(TOML_CFLAGS)
USER_LDFLAGS := $(LDFLAGS)
LDFLAGS := $(LLVM_LDFLAGS) $(USER_LDFLAGS)
LDLIBS += -lclang-cpp $(LLVM_LIBS) $(SQLITE_LIBS)
DEBUG_FLAG ?= -D_DEBUG_ -O0
RELEASE_FLAGS ?= -O2

TARGET = build/demo
SRC_DIR = src
OBJ_DIR = build/obj
INCLUDE_DIR = include
SCRIPT_DIR = scripts

# 将源文件分为两组：需要LLVM标志的和不需要的
LLVM_SRCS = $(wildcard $(SRC_DIR)/core/*.cc \
					   $(SRC_DIR)/util/key_generator/*.cc \
                       $(SRC_DIR)/core/processor/*.cc)

NORMAL_SRCS = $(wildcard $(SRC_DIR)/*.cc \
                         $(SRC_DIR)/interface/*.cc \
                         $(SRC_DIR)/util/*.cc \
                         $(SRC_DIR)/db/*.cc \
                         $(SRC_DIR)/model/*/*.cc)

# 生成对应的目标文件路径
LLVM_OBJS = $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(LLVM_SRCS))
NORMAL_OBJS = $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(NORMAL_SRCS))
ALL_OBJS = $(LLVM_OBJS) $(NORMAL_OBJS)

# ==============================================
# Build Targets
# ==============================================
all: $(TARGET)

$(TARGET): $(ALL_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(ALL_OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

# 使用LLVM标志编译核心文件
$(OBJ_DIR)/core/%.o: $(SRC_DIR)/core/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(COMMON_CXXFLAGS) -MMD -MP -c $< -o $@

# ==============================================
# Code Generation
# ==============================================
src/db/storage_facade_instantiations.inc: $(wildcard include/model/db/*.h) $(SCRIPT_DIR)/generate_instantiations.py
	$(PY) $(SCRIPT_DIR)/generate_instantiations.py

# 统一使用 LLVM 标志编译所有文件以防 ABI 不一致
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(COMMON_CXXFLAGS) -MMD -MP -c $< -o $@

# 使目标文件依赖于生成的实例化代码
$(OBJ_DIR)/db/storage_facade.o: src/db/storage_facade_instantiations.inc

-include $(ALL_OBJS:.o=.d)

# ==============================================
# Build Variants
# ==============================================
debug: CXXFLAGS += $(DEBUG_FLAG)
debug: all

release: CXXFLAGS += $(RELEASE_FLAGS)
release: all

# ==============================================
# Cleanup
# ==============================================
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# ==============================================
# Run & Help
# ==============================================
run: CXXFLAGS += $(DEBUG_FLAG)
run: $(TARGET)
	./$(TARGET) -c config.example.toml -s tests/slight-case.cc -o tests/ast.db

print-toolchain:
	@echo "LLVM_CONFIG=$(LLVM_CONFIG)"
	@echo "LLVM_VERSION=$(LLVM_VERSION)"
	@echo "LLVM_BINDIR=$(LLVM_BINDIR)"
	@echo "LLVM_LIBDIR=$(LLVM_LIBDIR)"
	@echo "CXX=$(CXX)"
	@echo "CXX_VERSION=$(CXX_VERSION)"
	@echo "LLVM_CXXFLAGS=$(LLVM_CXXFLAGS)"
	@echo "LLVM_LDFLAGS=$(LLVM_LDFLAGS)"
	@echo "LLVM_LIBS=$(LLVM_LIBS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "LDLIBS=$(LDLIBS)"
	@echo "SQLITE_CFLAGS=$(SQLITE_CFLAGS)"
	@echo "SQLITE_LIBS=$(SQLITE_LIBS)"
	@echo "TOML_CFLAGS=$(TOML_CFLAGS)"

# ==============================================
# Help Information
# ==============================================
help:
	@echo "Usage:"
	@echo "  make          # Build"
	@echo "  make debug    # Build with debug flags"
	@echo "  make release  # Build with release flags"
	@echo "  make run      # Build and run to show testing output"
	@echo "  make print-toolchain # Show discovered LLVM/SQLite toolchain"
	@echo "  make clean    # Clean up all build files"

.PHONY: all clean help run debug release print-toolchain
