# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11

# SDL2 flags
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

# Source files
SOURCES = main.c background.c sequence.c input.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Executable name
TARGET = program

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(SDL_LDFLAGS)
	@echo "Build complete! Run with ./$(TARGET)"

# Compile source files to object files
%.o: %.c header.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleaned build files"

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
