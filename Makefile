CXX ?= g++
CXXFLAGS ?= -Wall -Wextra -pedantic -Iinclude -g

PY = python3

# 使用llvm-config获取编译标志和链接标志
LLVM_CONFIG ?= llvm-config
LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags)

CLANG_LDFLAGS = $(shell $(LLVM_CONFIG) --libs core) -lclang-cpp

LDFLAGS ?= $(CLANG_LDFLAGS) $(LLVM_LDFLAGS) -lsqlite3
DEBUG_FLAG ?= -D_DEBUG_ -O0
RELEASE_FLAGS ?= -O2

TARGET = build/demo
SRC_DIR = src
OBJ_DIR = build/obj
INCLUDE_DIR = include
SCRIPT_DIR = scripts

# 将源文件分为两组：需要LLVM标志的和不需要的
LLVM_SRCS = $(wildcard $(SRC_DIR)/core/*.cc \
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

all: $(TARGET)

$(TARGET): $(ALL_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) -o $@ $^ $(LDFLAGS)

# 使用LLVM标志编译核心文件
$(OBJ_DIR)/core/%.o: $(SRC_DIR)/core/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) -MMD -MP -c $< -o $@

# 生成实例化代码
src/db/storage_facade_instantiations.inc: $(wildcard include/model/db/*.h) $(SCRIPT_DIR)/generate_instantiations.py
	$(PY) $(SCRIPT_DIR)/generate_instantiations.py

# 不使用LLVM标志编译其他文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# 使目标文件依赖于生成的实例化代码
$(OBJ_DIR)/db/storage_facade.o: src/db/storage_facade_instantiations.inc

-include $(ALL_OBJS:.o=.d)

debug: CXXFLAGS += $(DEBUG_FLAG)
debug: all

release: CXXFLAGS += $(RELEASE_FLAGS)
release: all

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

run: CXXFLAGS += $(DEBUG_FLAG)
run: $(TARGET)
	./$(TARGET) -c config.example.toml -s tests/slight-case.cc -o tests/ast.db

help:
	@echo "Usage:"
	@echo "  make          # Build"
	@echo "  make debug    # Build with debug flags"
	@echo "  make release  # Build with release flags"
	@echo "  make run      # Build and run to show testing output"
	@echo "  make clean    # Clean up all build files"

.PHONY: all clean help run debug release
