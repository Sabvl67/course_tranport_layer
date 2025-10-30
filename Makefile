# =============================
# Compiler and flags
# =============================
CXX       := g++
CXXFLAGS  := -Wall -O2 -MMD -MP
LDFLAGS   := -lX11
CXX_INC   := -Iinclude -Itest

# =============================
# Directories
# =============================
SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build

# =============================
# Targets and source discovery
# Add targets once you start building the project
# =============================
TARGETS   := 

# Find all .cc files in src/ (excluding test sources)
SRC_FILES := $(wildcard $(SRC_DIR)/*.cc)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Find all test .cc files
TEST_FILES := $(wildcard $(TEST_DIR)/*.cc)
TEST_OBJ   := $(patsubst $(TEST_DIR)/%.cc,$(BUILD_DIR)/test_%.o,$(TEST_FILES))
TEST_BIN   := $(BUILD_DIR)/tests

# =============================
# Default rule
# =============================
all: $(TARGETS)

# =============================
# Link step for programs
# =============================
$(TARGETS):
	$(CXX) $^ -o $@ $(LDFLAGS)

# =============================
# Compile rules
# =============================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXX_INC) -c $< -o $@

$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXX_INC) -c $< -o $@

# =============================
# Test target (Google Test)
# =============================
test: $(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) $(OBJ_FILES)
	$(CXX) $^ -o $@ -lgtest -lgtest_main -pthread $(LDFLAGS)

# =============================
# Include dependency files
# =============================
-include $(wildcard $(BUILD_DIR)/*.d)

# =============================
# Directory creation
# =============================
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# =============================
# Cleaning
# =============================
clean:
	rm -rf $(BUILD_DIR) $(TARGETS)

.PHONY: all clean test
