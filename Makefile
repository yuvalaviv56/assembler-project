# Makefile for Assembler Project
CC = gcc
CFLAGS = -Wall -ansi -pedantic -Iinclude
TARGET = assembler

# Source files
SRCS = src/assembler.c \
       src/string_utils.c \
       src/errors.c \
       src/macro.c \
       src/symbol_table.c \
       src/parser.c \
       src/first_pass.c \
       src/second_pass.c \
       src/output.c \
       src/file_handler.c \
       src/operand_encoder.c

# Object files
OBJS = $(SRCS:.c=.o)

# Header files
HEADERS = include/constants.h \
          include/structures.h \
          include/string_utils.h \
          include/errors.h \
          include/macro.h \
          include/symbol_table.h \
          include/parser.h \
          include/first_pass.h \
          include/second_pass.h \
          include/output.h \
          include/file_handler.h \
          include/operand_encoder.h \
          include/globals.h

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
	rm -f *.am *.ob *.ent *.ext
	rm -f tests/*.am tests/*.ob tests/*.ent tests/*.ext