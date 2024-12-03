# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lm

# Directories
SRC_DIR = src
BUILD_DIR = build

# Target executable
TARGET = raytracer

# Source files discovery
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# Default target
all: $(BUILD_DIR)/$(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile with PNG support
png: CFLAGS += -DUSE_PNG
png: $(BUILD_DIR)/$(TARGET)

# Compile with PPM support
ppm: CFLAGS += -DUSE_PPM
ppm: $(BUILD_DIR)/$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(BUILD_DIR)/$(TARGET)

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: $(BUILD_DIR)/$(TARGET)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Link the executable
$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Include dependency files
-include $(DEPS)

# Install the executable
install: $(BUILD_DIR)/$(TARGET)
	install -m 755 $(BUILD_DIR)/$(TARGET) /usr/local/bin/$(TARGET)

# Uninstall the executable
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Format source files
format:
	find $(SRC_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)

# Phony targets
.PHONY: all png ppm debug release clean install uninstall format

# Default target when no arguments provided
.DEFAULT_GOAL := all
