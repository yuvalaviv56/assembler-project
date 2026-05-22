/*
 * structures.h
 * Defines all data structures used in the assembler
 */

#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "constants.h"

/* Symbol attributes (can be combined with bitwise OR) */
typedef enum {
    ATTR_NONE = 0,      /* No attributes set - default state */
    ATTR_CODE = 1,      /* Symbol defined in code section */
    ATTR_DATA = 2,      /* Symbol defined in data section */
    ATTR_ENTRY = 4,     /* Symbol marked as entry - referencable by other files */
    ATTR_EXTERNAL = 8   /* Symbol marked as external - defined in another file */
} SymbolAttribute;

/* Symbol table entry (linked list node) */
typedef struct Symbol {
    char name[MAX_LABEL_LENGTH + 1];  /* Symbol name */
    int value;                         /* Address/value */
    unsigned int attributes;           /* Combination of the appropriate SymbolAttribute flags */
    struct Symbol *next;               /* Pointer to next symbol in linked list */
} Symbol;

/* Symbol table */
typedef struct {
    Symbol *head;   /* Head of symbol linked list */
    int count;      /* Number of symbols in the table */
} SymbolTable;

/* Memory word (12 bits + 3 bits for A/R/E) */
typedef struct {
    unsigned int word : 12;     /* 12-bit word value */
    unsigned int are : 3;       /* A/R/E bits (Absolute/Relocatable/External) */
} MemoryWord;

/* Code and data images */
typedef struct {
    MemoryWord code[MEMORY_SIZE];   /* Code section */
    MemoryWord data[MEMORY_SIZE];   /* Data section */
    int IC;                         /* Instruction counter */
    int DC;                         /* Data counter */
    int ICF;                        /* Final IC value */
    int DCF;                        /* Final DC value */
} MemoryImage;

/* External reference (for .ext file) */
typedef struct ExternalRef {
    char symbol[MAX_LABEL_LENGTH + 1];  /* External symbol name (+1 for null terminator) */
    int address;                         /* Address where symbol is used */
    struct ExternalRef *next;            /* Pointer to next reference */
} ExternalRef;

/* External references list */
typedef struct {
    ExternalRef *head;  /* Head of references list */
    int count;          /* Number of references recorded */
} ExternalList;

/* Line type */
typedef enum {
    LINE_EMPTY,         /* Empty line or comment */
    LINE_DIRECTIVE,     /* Directive (.data, .string, etc.) */
    LINE_INSTRUCTION,   /* Instruction (mov, add, etc.) */
    LINE_ERROR          /* parsing error */
} LineType;

/* Addressing modes */
typedef enum {
    MODE_IMMEDIATE = 0,     /* #N - for immediate values*/
    MODE_DIRECT = 1,        /* LABEL - for direct addressing*/
    MODE_RELATIVE = 2,      /* %LABEL - for relative addressing */
    MODE_REGISTER = 3       /* rN - for register addressing */
} AddressingMode;

/* Operand information */
typedef struct {
    AddressingMode mode;                /* Addressing mode */
    int value;                          /* Immediate value or register number */
    char symbol[MAX_LABEL_LENGTH + 1];  /* Symbol name - for direct or relative addressing */
} Operand;

#endif /* STRUCTURES_H */