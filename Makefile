# =============================
# Compiler and flags
# =============================
CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -O2 -MMD -MP -I.
LDFLAGS   := -lz
CXX_INC   := -Iinclude -Itest -I/opt/homebrew/opt/googletest/include
GTEST_LIB := -L/opt/homebrew/opt/googletest/lib

# =============================
# Directories
# =============================
SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build

# =============================
# Targets and source discovery
# =============================
TARGETS   := ctp_sender ctp_receiver

# Specific executables
SENDER    := ctp_sender
RECEIVER  := ctp_receiver 

# Find all .cc files in src/ (excluding test sources)
SRC_FILES := $(wildcard $(SRC_DIR)/*.cc)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRC_FILES))

# For tests, exclude main.cc
TEST_SRC_FILES := $(filter-out $(SRC_DIR)/main.cc,$(SRC_FILES))
TEST_SRC_OBJ := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(TEST_SRC_FILES))

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
$(SENDER): $(SRC_DIR)/sender.cc
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Sender compiled: ./$(SENDER)"

$(RECEIVER): $(SRC_DIR)/ctp_receiver.cc
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Receiver compiled: ./$(RECEIVER)"

# Convenience targets
sender: $(SENDER)
receiver: $(RECEIVER)

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

$(TEST_BIN): $(TEST_OBJ) $(TEST_SRC_OBJ)
	$(CXX) $^ -o $@ $(GTEST_LIB) -lgtest -lgtest_main -pthread

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
