/*
 * constants.h
 * Defines all constant values used in the assembler
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

/* Memory and size */
#define MEMORY_SIZE 4096        /* Total memory size in words */
#define WORD_SIZE 12            /* single word size in bits */
#define MAX_LINE_LENGTH 81      /* Maximum line length (80 + '\n') */
#define MAX_LABEL_LENGTH 31     /* Maximum label name length */
#define INITIAL_IC 100          /* Starting address for code */
#define INITIAL_DC 0            /* Starting address for data */

/* File extensions */
#define EXT_SOURCE ".as"        /* Source file */
#define EXT_MACRO ".am"         /* After macro expansion */
#define EXT_OBJECT ".ob"        /* Object file */
#define EXT_ENTRIES ".ent"      /* Entries file */
#define EXT_EXTERNALS ".ext"    /* Externals file */

/* Registers */
#define NUM_REGISTERS 8         /* number of registers (r0-r7) */
#define REGISTER_PREFIX 'r'     /* First character of all register names (r0-r7) */

/* Addressing modes */
#define ADDR_IMMEDIATE 0        /* #N - for immediate values */
#define ADDR_DIRECT 1           /* LABEL - for direct addressing */
#define ADDR_RELATIVE 2         /* %LABEL - for relative addressing */
#define ADDR_REGISTER 3         /* rN - for register addressing */

/* Special characters */
#define IMMEDIATE_PREFIX '#'
#define RELATIVE_PREFIX '%'
#define LABEL_END ':'
#define COMMENT_CHAR ';'
#define STRING_QUOTE '"'
#define OPERAND_SEPARATOR ','
#define DIRECTIVE_PREFIX '.'

/* Directives */
#define DIR_DATA ".data" /* Store int values in the data image */
#define DIR_STRING ".string"  /* Store a string as ASCII codes in the data image */
#define DIR_ENTRY ".entry" /* Export a label so other files can reference it */
#define DIR_EXTERN ".extern" /* Declare a label that is defined in another file */

/* Macro keywords */
#define MACRO_START "mcro"
#define MACRO_END "mcroend"

/* Operation codes (opcode values) */
#define OP_MOV 0
#define OP_CMP 1
#define OP_ADD 2
#define OP_SUB 2
#define OP_LEA 4
#define OP_CLR 5
#define OP_NOT 5
#define OP_INC 5
#define OP_DEC 5
#define OP_JMP 9
#define OP_BNE 9
#define OP_JSR 9
#define OP_RED 12
#define OP_PRN 13
#define OP_RTS 14
#define OP_STOP 15

/* Function codes (funct values) */
#define FUNCT_NONE 0
#define FUNCT_ADD 10
#define FUNCT_SUB 11
#define FUNCT_CLR 10
#define FUNCT_NOT 11
#define FUNCT_INC 12
#define FUNCT_DEC 13
#define FUNCT_JMP 10
#define FUNCT_BNE 11
#define FUNCT_JSR 12

/* A/R/E encoding (instructions for linker/loader) */
#define ARE_ABSOLUTE 0 /* Fixed value - load-address independent */
#define ARE_EXTERNAL 1 /* symbol defined in another file - filled in by the linker */
#define ARE_RELOCATABLE 2 /*symbol in this file - may need adjustment by the loader */

/* Error and success codes */
#define SUCCESS 0
#define ERROR 1

/* Maximum values */
#define MAX_MACROS 100
#define MAX_SYMBOLS 500
#define MAX_MACRO_LINES 50

#endif /* CONSTANTS_H */