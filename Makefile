# Makefile for Assembler Project
# Complies with C90 standard with strict warnings

CC = gcc
CFLAGS = -Wall -ansi -pedantic -Iinclude
TARGET = assembler

# Source files
SRCS = src/main.c \
       src/string_utils.c \
       src/errors.c \
       src/macro.c \
       src/parser.c \
       src/symbol_table.c \
       src/first_pass.c \
       src/second_pass.c \
       src/operand_encoder.c \
       src/output.c \
       src/file_handler.c

# Object files (replace .c with .o)
OBJS = $(SRCS:.c=.o)

# Header files
HEADERS = include/constants.h \
          include/structures.h \
          include/globals.h \
          include/string_utils.h \
          include/errors.h \
          include/macro.h \
          include/parser.h \
          include/symbol_table.h \
          include/first_pass.h \
          include/second_pass.h \
          include/operand_encoder.h \
          include/output.h \
          include/file_handler.h

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile source files to object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)
	rm -f tests/*.am tests/*.ob tests/*.ent tests/*.ext

# Clean and rebuild
rebuild: clean all

# Run tests (add test commands here later)
test: $(TARGET)
	@echo "Running tests..."
	@echo "Test infrastructure coming soon"

.PHONY: all clean rebuild test