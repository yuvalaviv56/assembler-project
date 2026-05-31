C ASSEMBLE PROJECT - COURSE 20465

This is our final project for the System Programming Lab course. It is a two-pass assembler written in ANSI C (C90 standard). The program takes assembly source files, expands macros, checks for syntax errors, and translates everything into machine code for a 12-bit virtual computer.

WHO WROTE THIS:
Yair Geri and Yuval Aviv

Architecture info:
8 general registers: r0-r7.
12-bit memory words (total of 4096 memory addresses).
Negative numbers use the 2's complement method.

How the code works:
Step 1: Pre-Assembler (Macros)
The program reads the ".as" file and finds all the macros. It replaces the macro names with the real lines of code and saves a new file with a ".am" extension.

Step 2: First Pass
We read the ".am" file line by line, manage the counters (IC starts at 100, DC starts at 0) and calculate how much memory each line needs. We build the Symbol Table using a dynamic linked list, and encode the first word of the instructions.

Step 3: Second Pass
We go over the file for a second time to fill the missing gaps. Now that we have all the symbols in our table, we calculate the real addresses for labels, jumps, and connect external/entry definitions.

Step 4: Output Files
If there are any syntax errors in the first or second pass, the program prints all of them with the line numbers so the user can fix them. If the code has no errors, the assembler creates the final output files (.am,.ob,.ent,.ext).

The 4 addressing modes we support:
Mode 0 (Immediate):For numbers starting with '#', like #-5
Mode 1 (Direct): For variables and labels, Example: LOOP.
Mode 2 (Relative): Jumps starting with '%', like %END.
Mode 3 (Register): Direct register access from r0 to r7.

How to compile and build:
Everything is managed by a Makefile using strict flags (-Wall -ansi -pedantic).

To build the project and run it:
make

To clean all object files and old outputs:
make clean

How to run the program:
Execute the assembler by passing the target filenames as arguments.

NOTE: Do not write the ".as" extension when running the program.
Example: ./assembler file1 file2

Produced files after a successful run
file.am : The source file after macro expansion.
file.ob : The object file containing memory addresses and encoded machine code.
file.ent: Entry symbols table (Created only if .entry is used in the code).
file.ext: External symbols reference table (Created only if .extern is used in the code).