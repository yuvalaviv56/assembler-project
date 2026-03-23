# Makefile for Assembler Project

CC = gcc
CFLAGS = -Wall -ansi -pedantic -Iinclude
TARGET = assembler
TEST_SYMBOL = test_symbol_table
TEST_PARSER = test_parser

# Source files (only implemented ones)
SRCS = src/main.c \
       src/string_utils.c \
       src/errors.c \
       src/macro.c \
       src/symbol_table.c \
       src/parser.c\
       src/first_pass.c

# Object files
OBJS = $(SRCS:.c=.o)

# Header files
HEADERS = include/constants.h \
          include/structures.h \
          include/string_utils.h \
          include/errors.h \
          include/macro.h \
          include/symbol_table.h \
          include/parser.h\
          include/first_pass.h

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile source files to object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Build and run symbol table test
test_symbol: test_symbol_table.c src/symbol_table.c src/string_utils.c
	$(CC) $(CFLAGS) -o $(TEST_SYMBOL) test_symbol_table.c src/symbol_table.c src/string_utils.c
	@echo ""
	@echo "Running symbol table tests..."
	@echo ""
	./$(TEST_SYMBOL)

# Build and run parser test
test_parser: test_parser.c src/parser.c src/string_utils.c
	$(CC) $(CFLAGS) -o $(TEST_PARSER) test_parser.c src/parser.c src/string_utils.c
	@echo ""
	@echo "Running parser tests..."
	@echo ""
	./$(TEST_PARSER)

# Run all tests
test_all: test_symbol test_parser

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_SYMBOL) $(TEST_PARSER)
	rm -f tests/valid/*.am tests/valid/*.ob tests/valid/*.ent tests/valid/*.ext
	rm -f tests/errors/*.am tests/errors/*.ob tests/errors/*.ent tests/errors/*.ext
	rm -f *.am *.ob *.ent *.ext

# Clean and rebuild
rebuild: clean all

# Run assembler tests
test: $(TARGET)
	@echo "Running macro expansion tests..."
	./$(TARGET) tests/valid/test_macro

.PHONY: all clean rebuild test test_symbol test_parser test_all