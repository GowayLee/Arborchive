CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -Iinclude
LDFLAGS ?= -lclang -lsqlite3
DEBUG_FLAG ?= -D_DEBUG_ -g -O0
RELEASE_FLAGS ?= -O2

TARGET = build/demo
SRC_DIR = src
OBJ_DIR = build/obj
INCLUDE_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.cc \
                  $(SRC_DIR)/interface/*.cc \
                  $(SRC_DIR)/util/*.cc \
                  $(SRC_DIR)/db/*.cc \
                  $(SRC_DIR)/db/table_defines/*.cc \
                  $(SRC_DIR)/core/*.cc \
                  $(SRC_DIR)/core/processor/*.cc \
                  $(SRC_DIR)/model/*/*.cc)

OBJS = $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(OBJS:.o=.d)

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
