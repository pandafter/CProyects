# Voxel Engine Makefile - Refactored Architecture
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
INCLUDES = -Iinclude
LIBS = -lopengl32 -lglu32 -lgdi32 -luser32 -lkernel32

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Source files by category
CORE_SOURCES = $(SRC_DIR)/core/memory.c $(SRC_DIR)/core/math3d.c $(SRC_DIR)/core/input.c
GRAPHICS_SOURCES = $(SRC_DIR)/graphics/renderer.c $(SRC_DIR)/graphics/window.c
GRAPHICS_UI_SOURCES = $(SRC_DIR)/graphics/ui/menu.c
GRAPHICS_OPENGL_SOURCES = $(SRC_DIR)/graphics/opengl/simple_opengl.c
GRAPHICS_SHADER_SOURCES = $(SRC_DIR)/graphics/shaders/shaders.c
GRAPHICS_EFFECTS_SOURCES = $(SRC_DIR)/graphics/effects/Skybox.c $(SRC_DIR)/graphics/effects/Shadow.c $(SRC_DIR)/graphics/effects/Volumetrics.c
WORLD_SOURCES = $(SRC_DIR)/world/chunk_system.c
MAIN_SOURCE = $(SRC_DIR)/main.c

# Object files
CORE_OBJECTS = $(CORE_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
GRAPHICS_OBJECTS = $(GRAPHICS_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
GRAPHICS_UI_OBJECTS = $(GRAPHICS_UI_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
GRAPHICS_OPENGL_OBJECTS = $(GRAPHICS_OPENGL_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
GRAPHICS_SHADER_OBJECTS = $(GRAPHICS_SHADER_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
GRAPHICS_EFFECTS_OBJECTS = $(GRAPHICS_EFFECTS_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
WORLD_OBJECTS = $(WORLD_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
MAIN_OBJECT = $(BUILD_DIR)/main.o

# All objects
OBJECTS = $(CORE_OBJECTS) $(GRAPHICS_OBJECTS) $(GRAPHICS_UI_OBJECTS) $(GRAPHICS_OPENGL_OBJECTS) $(GRAPHICS_SHADER_OBJECTS) $(GRAPHICS_EFFECTS_OBJECTS) $(WORLD_OBJECTS) $(MAIN_OBJECT)

# Target executable
TARGET = voxel_engine.exe

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory structure
$(BUILD_DIR):
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@if not exist $(BUILD_DIR)\core mkdir $(BUILD_DIR)\core
	@if not exist $(BUILD_DIR)\graphics mkdir $(BUILD_DIR)\graphics
	@if not exist $(BUILD_DIR)\graphics\ui mkdir $(BUILD_DIR)\graphics\ui
	@if not exist $(BUILD_DIR)\graphics\opengl mkdir $(BUILD_DIR)\graphics\opengl
	@if not exist $(BUILD_DIR)\graphics\shaders mkdir $(BUILD_DIR)\graphics\shaders
	@if not exist $(BUILD_DIR)\graphics\effects mkdir $(BUILD_DIR)\graphics\effects
	@if not exist $(BUILD_DIR)\world mkdir $(BUILD_DIR)\world

# Build executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)
	@echo "Build complete: $(TARGET)"

# Compile core sources
$(BUILD_DIR)/core/%.o: $(SRC_DIR)/core/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile graphics sources
$(BUILD_DIR)/graphics/%.o: $(SRC_DIR)/graphics/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile graphics/ui sources
$(BUILD_DIR)/graphics/ui/%.o: $(SRC_DIR)/graphics/ui/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile graphics/opengl sources
$(BUILD_DIR)/graphics/opengl/%.o: $(SRC_DIR)/graphics/opengl/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile graphics/shaders sources
$(BUILD_DIR)/graphics/shaders/%.o: $(SRC_DIR)/graphics/shaders/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile graphics/effects sources
$(BUILD_DIR)/graphics/effects/%.o: $(SRC_DIR)/graphics/effects/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile world sources
$(BUILD_DIR)/world/%.o: $(SRC_DIR)/world/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile main source
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	@echo "Cleaning build files..."
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@if exist $(TARGET) del $(TARGET)
	@echo "Clean complete"

# Run the executable
run: $(TARGET)
	@echo "Running $(TARGET)..."
	$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(BUILD_DIR) $(TARGET)

# Release build
release: CFLAGS += -O3 -DNDEBUG
release: $(BUILD_DIR) $(TARGET)

# Help
help:
	@echo "Available targets:"
	@echo "  all      - Build the executable (default)"
	@echo "  clean    - Remove build files"
	@echo "  run      - Build and run the executable"
	@echo "  debug    - Build with debug symbols"
	@echo "  release  - Build optimized release version"
	@echo "  help     - Show this help message"

# Phony targets
.PHONY: all clean run debug release help