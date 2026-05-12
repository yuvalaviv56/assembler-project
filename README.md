# Assembler Project - Course 20465

Two-pass assembler for a custom assembly language, written in C90.

## Authors
- Yair Geri
- Yuval Aviv

## Project Structure

assembler-project/
├── src/           # Source files
├── include/       # Header files
├── tests/         # Test files
└── Makefile       # Build configuration

## Building

make clean
make

## Usage

./assembler file1 file2 file3

Files should be provided without the .as extension.

## Output Files
- .am - After macro expansion
- .ob - Object file (machine code)
- .ent - Entry symbols
- .ext - External symbols

## Features
- Macro expansion
- Two-pass assembly
- Symbol table management
- Multiple addressing modes
- Error detection and reporting

## Standards
- C90 compliant
- Compiled with -Wall -ansi -pedantic