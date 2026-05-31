/*
 * first_pass.c
 * First pass of the assembler - first of two scans over the post-macro-expansion (.am) file.
 *
 *First pass responsibilities:
 * Build the symbol table, 
 * store the properties of the first word of each instruction into the code image as a 12-bit word (opcode, funct, addressing modes),
 * compute and store operand words whose values are known immediately (immediate values and register numbers), 
 * reserve space in the code image for values filled in by the second pass, 
 * store .data and .string values in the data image and track IC and DC to produce the final memory layout.
 *
 *
 * Note: .entry directives are skipped here and handled in the second pass,
 * since a label may be used before it is defined in the file.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "first_pass.h"
#include "parser.h"
#include "string_utils.h"
#include "errors.h"
#include "constants.h"
#include "globals.h"

/* operation table entry - matches operation name to its properties */
typedef struct {
    const char *name;   /* operation name */
    int opcode;    /* opcode value (bits 11-8 of first word) */
    int funct;    /* funct value (bits 7-4 of first word), 0 if unused */
    int num_operands;   /* number of operands the operation expects (0, 1, or 2) */
} OperationInfo;

/* static table mapping each operation name to its appropriate opcode, funct, and operand count values*/
static const OperationInfo OPERATIONS[] = {
    {"mov",  0,  0,  2},
    {"cmp",  1,  0,  2},
    {"add",  2,  10, 2},
    {"sub",  2,  11, 2},
    {"lea",  4,  0,  2},
    {"clr",  5,  10, 1},
    {"not",  5,  11, 1},
    {"inc",  5,  12, 1},
    {"dec",  5,  13, 1},
    {"jmp",  9,  10, 1},
    {"bne",  9,  11, 1},
    {"jsr",  9,  12, 1},
    {"red",  12, 0,  1},
    {"prn",  13, 0,  1},
    {"rts",  14, 0,  0},
    {"stop", 15, 0,  0},
    {NULL,   0,  0,  0}
};

/*
 * looks up an operation by name and retrieves the details of its properties
 * input: operation name string, pointers to store the opcode, funct, and operand count values (any can be NULL)
 * output: true if operation found, false otherwise
 */
int get_operation_info(const char *operation, int *opcode, int *funct, int *num_operands) {
    int i;

    if (operation == NULL) {
        return FALSE;
    }

    for (i = 0; OPERATIONS[i].name != NULL; i++) {
        if (strcmp(operation, OPERATIONS[i].name) == 0) {
            if (opcode) *opcode = OPERATIONS[i].opcode;
            if (funct) *funct = OPERATIONS[i].funct;
            if (num_operands) *num_operands = OPERATIONS[i].num_operands;
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * calculates the number of memory words an instruction occupies
 * input: operation name, source operand string, destination operand string
 * output: number of words (1 for no operands, and up to 3 for two operands)
 */
int calculate_instruction_length(const char *operation, const char *source, const char *dest) {
    int length = 1;     /* word count (first word always included) */
    int num_operands;   /* number of operands expected by this operation */


    if (!get_operation_info(operation, NULL, NULL, &num_operands)) {
        return 1;
    }

    if (num_operands == 0) {
        return 1;
    }

    if (num_operands == 1 && dest != NULL) {
        length++;
        return length;
    }

    
    if (num_operands == 2 && source != NULL && dest != NULL) {
        length += 2; 
    }

    return length;
}

/*
 * encodes and stores the first word of an instruction into the code image in a 12 bit format
 * input: memory image, operation name, source operand, destination operand
 */
static void encode_first_word(MemoryImage *memory, const char *operation,
                              const char *source, const char *dest) {
    int opcode, funct, num_operands;    /* operation properties from the static table */
    unsigned int word = 0;      /* initialize int to hold the bitwise representation of the word */
    AddressingMode src_mode = MODE_IMMEDIATE, dest_mode = MODE_IMMEDIATE;   /* default addressing modes */

     /* making sure the input operation is a valid operation */
    if (!get_operation_info(operation, &opcode, &funct, &num_operands)) {
        return;
    }

    word |= ((unsigned int)opcode & 0xF) << 8;

    word |= ((unsigned int)funct & 0xF) << 4;

    if (num_operands >= 1 && dest != NULL) {
        dest_mode = identify_addressing_mode(dest);
        word |= ((unsigned int)dest_mode & 0x3) << 0;
    }

    if (num_operands == 2 && source != NULL) {
        src_mode = identify_addressing_mode(source);
        word |= ((unsigned int)src_mode & 0x3) << 2;
    }

    memory->code[memory->IC].word = word & 0xFFF;
    memory->code[memory->IC].are = ARE_ABSOLUTE;
}

/*
 * validates an operand and detects invalid register names like 'r8' (error detection)
 * input: operand string, line number (for error reporting)
 * output: success if valid, error if invalid register detected
 */
static int validate_operand(const char *operand, int line_num) {
    int reg_num; /* temp variable for parse_register */

    if (operand == NULL || *operand == '\0') return SUCCESS;

    /* Check for register syntax with invalid number like 'r8' */
    if (operand[0] == REGISTER_PREFIX && isdigit((unsigned char)operand[1])) {
        if (!parse_register(operand, &reg_num)) {
            print_error(line_num, ERR_INVALID_REGISTER, operand);
            return ERROR;
        }
    }

    return SUCCESS;
}

/*
 * handles a .data directive by storing the accompanying int values in the data image
 * input: parameter string (comma-separated integers), line number (for error reporting), memory image
 * output: success if all values stored, error if invalid input or memory overflow
 */
int process_data_directive(const char *params, int line_num, MemoryImage *memory) {
    char params_copy[MAX_LINE_LENGTH];      /* a copy of the parameters for strtok to break into manageable tokens */
    char *token;        /* pointer to the token currently being handled */
    int value;      /* integer value of the current token being handled */

    /* make sure the directive actually has parameters (error detection) */
    if (params == NULL || *params == '\0') {
        print_error(line_num, ERR_EMPTY_DIRECTIVE, NULL);
        return ERROR;
    }

    safe_strcpy(params_copy, params, MAX_LINE_LENGTH);

    /* initialize the token pointer with the first token and store each comma separated value */
    token = strtok(params_copy, ",");
    while (token != NULL) {
        trim(token);

        /* make sure current token's value is a valid int value (error detection) */
        if (!parse_integer(token, &value)) {
            print_error(line_num, ERR_INVALID_NUMBER, token);
            return ERROR;
        }

        /* make sure data image has available space (error detection) */
        if (memory->DC >= MEMORY_SIZE) {
            print_error(line_num, ERR_MEMORY_OVERFLOW, NULL);
            return ERROR;
        }

        memory->data[memory->DC].word = value & 0xFFF;
        memory->data[memory->DC].are = ARE_ABSOLUTE;
        memory->DC++;

        token = strtok(NULL, ",");
    }

    return SUCCESS;
}

/*
 * handles a .string directive by storing the ascii codes of each character in the data image
 * input: parameter string (quoted string), line number (for error reporting), memory image
 * output: success if string stored, error if missing quotes or memory overflow
 */
int process_string_directive(const char *params, int line_num, MemoryImage *memory) {
    const char *p;      /*pointer to the current char being handled */
    int in_quotes = FALSE;  /* flag for whether we are currently inside quotes (error detection) */

    /* make sure the directive actually has parameters (error detection) */
    if (params == NULL || *params == '\0') {
        print_error(line_num, ERR_EMPTY_DIRECTIVE, NULL);
        return ERROR;
    }

    p = skip_whitespace(params);

    /* make sure the string starts with a quote character (error detection) */
    if (*p != STRING_QUOTE) {
        print_error(line_num, ERR_MISSING_QUOTE, NULL);
        return ERROR;
    }

    p++;
    in_quotes = TRUE;

    /* scan through string's characters and store their asci value in the data image */
    while (*p != '\0' && *p != '\n') {
        if (*p == STRING_QUOTE) {
            in_quotes = FALSE;
            break;
        }

        /* make sure data image has available space for the current character (error detection) */
        if (memory->DC >= MEMORY_SIZE) {
            print_error(line_num, ERR_MEMORY_OVERFLOW, NULL);
            return ERROR;
        }

        memory->data[memory->DC].word = (unsigned int)(*p) & 0xFFF;
        memory->data[memory->DC].are = ARE_ABSOLUTE;
        memory->DC++;

        p++;
    }

    /* make sure the string ends with a quote character (error detection) */
    if (in_quotes) {
        print_error(line_num, ERR_MISSING_QUOTE, NULL);
        return ERROR;
    }

    if (memory->DC >= MEMORY_SIZE) {
        print_error(line_num, ERR_MEMORY_OVERFLOW, NULL);
        return ERROR;
    }

    memory->data[memory->DC].word = 0;
    memory->data[memory->DC].are = ARE_ABSOLUTE;
    memory->DC++;

    return SUCCESS;
}

/*
 * validates a .entry directive - actual handling is done during the second pass
 * input: parameter string (label name), line number (for error reporting)
 * output: success if parameter exists, error if empty
 */
int process_entry_directive(const char *params, int line_num) {
    /* make sure the directive actually has parameters (error detection) */
    if (params == NULL || *params == '\0') {
        print_error(line_num, ERR_EMPTY_DIRECTIVE, NULL);
        return ERROR;
    }

    return SUCCESS;
}

/*
 * handles a .extern directive by adding the external symbol to the symbol table
 * input: parameter string (label name), line number (for error reporting), symbol table
 * output: success if symbol added, error if invalid name or duplicate
 */
int process_extern_directive(const char *params, int line_num, SymbolTable *symbol_table) {
    char symbol_name[MAX_LABEL_LENGTH + 1]; /* buffer for external symbol name (+1 for null terminator) */

    /* make sure the directive actually has parameters (error detection) */
    if (params == NULL || *params == '\0') {
        print_error(line_num, ERR_EMPTY_DIRECTIVE, NULL);
        return ERROR;
    }

    safe_strcpy(symbol_name, params, MAX_LABEL_LENGTH + 1);
    trim(symbol_name);

    /* make sure the external symbol name is a valid label (error detection) */
    if (!is_valid_label(symbol_name)) {
        print_error(line_num, ERR_INVALID_LABEL, symbol_name);
        return ERROR;
    }

    /* add the external symbol to the symbol table with value 0 - error if symbol already exists */
    if (!symbol_table_add(symbol_table, symbol_name, 0, ATTR_EXTERNAL)) {
        print_error(line_num, ERR_DUPLICATE_LABEL, symbol_name);
        return ERROR;
    }

    return SUCCESS;
}

/*
 * routes a directive line to the appropriate handler based on its type
 * input: directive name, parameters, label, line number (for error reporting), symbol table, memory image
 * output: success if directive processed properly, error if invalid input or processing failed
 */
int process_directive(const char *directive, const char *params, const char *label,
                      int line_num, SymbolTable *symbol_table, MemoryImage *memory) {

    /* if a label exists handle it based on directive type */
    if (label != NULL && *label != '\0') {
        /*  there should be no label for .entry or .extern directives */
        if (strcmp(directive, DIR_ENTRY) == 0 || strcmp(directive, DIR_EXTERN) == 0) {
            print_error(line_num, ERR_NONE, "Label not allowed with .entry or .extern directives");
            return ERROR;
        }

        /* add label to symbol table with current DC as its address */
        if (!symbol_table_add(symbol_table, label, memory->DC, ATTR_DATA)) {
            print_error(line_num, ERR_DUPLICATE_LABEL, label);
            return ERROR;
        }
    }

    if (strcmp(directive, DIR_DATA) == 0) {
        return process_data_directive(params, line_num, memory);
    } else if (strcmp(directive, DIR_STRING) == 0) {
        return process_string_directive(params, line_num, memory);
    } else if (strcmp(directive, DIR_ENTRY) == 0) {
        return process_entry_directive(params, line_num);
    } else if (strcmp(directive, DIR_EXTERN) == 0) {
        return process_extern_directive(params, line_num, symbol_table);
    } else {
        /* directive type was not recognized (error detection) */
        print_error(line_num, ERR_INVALID_DIRECTIVE, directive);
        return ERROR;
    }

}

/*
 * handles an instruction line by validating operands, encoding the first word and advancing the IC
 * input: operation name, operands string, label, line number (for error reporting), symbol table, memory image
 * output: success if instruction processed properly, error if invalid operands or memory overflow
 */
int process_instruction(const char *operation, const char *operands, const char *label,
                        int line_num, SymbolTable *symbol_table, MemoryImage *memory) {
    char source[MAX_LINE_LENGTH] = "";      /* source operand string */
    char dest[MAX_LINE_LENGTH] = "";       /* destination operand string */
    char operands_copy[MAX_LINE_LENGTH];    /* copy of operands string for parse_operands to work on without modifying the original */
    int num_operands_parsed;    /* actual number of operands found in the line being handled */
    int num_operands_expected;  /* number of operands the operation requires */
    int instruction_length;    /* number of words in the instruction */

    /* if a label exists add it to the symbol table with current IC as its address */
    if (label != NULL && *label != '\0') {
        /* add label to symbol table and make sure it's not already defined */
        if (!symbol_table_add(symbol_table, label, memory->IC, ATTR_CODE)) {
            print_error(line_num, ERR_DUPLICATE_LABEL, label);
            return ERROR;
        }
    }

    /* make sure the operation is valid based on the operation table and get back its expected operand count (error detection) */
    if (!get_operation_info(operation, NULL, NULL, &num_operands_expected)) {
        print_error(line_num, ERR_INVALID_INSTRUCTION, operation);
        return ERROR;
    }

    /* store a copy of the operands and count how many there are */
    safe_strcpy(operands_copy, operands, MAX_LINE_LENGTH);
    num_operands_parsed = parse_operands(operands_copy, source, dest, MAX_LINE_LENGTH);

    /* make sure the number of operands found matches what the operation expects (error detection) */
    if (num_operands_parsed != num_operands_expected) {
        print_error(line_num, ERR_OPERAND_COUNT, operation);
        return ERROR;
    }

    if (num_operands_expected == 2) {
        if (validate_operand(source, line_num) == ERROR) return ERROR;
    }
    if (num_operands_expected >= 1) {
        if (validate_operand(dest, line_num) == ERROR) return ERROR;
    }

    /* calculate the instruction's total word count based on operation and its operands */
    if (num_operands_expected == 2) {
        instruction_length = calculate_instruction_length(operation, source, dest);
    } else if (num_operands_expected == 1) {
        instruction_length = calculate_instruction_length(operation, NULL, dest);
    } else {
        instruction_length = calculate_instruction_length(operation, NULL, NULL);
    }

    /* make sure theres enough space in the code image for storing the instruction (error detection) */
    if (memory->IC + instruction_length > MEMORY_SIZE) {
        print_error(line_num, ERR_MEMORY_OVERFLOW, NULL);
        return ERROR;
    }

    if (num_operands_expected == 2) {
        encode_first_word(memory, operation, source, dest);
    } else if (num_operands_expected == 1) {
        encode_first_word(memory, operation, NULL, dest);
    } else {
        encode_first_word(memory, operation, NULL, NULL);
    }
                     

    memory->IC += instruction_length;

    return SUCCESS;
}

/*
 * the equvilant to the "main" function of the first pass - reads the .am file, builds the symbol table and encodes instruction first words into the code image
 * input: path to .am file, symbol table, memory image
 * output: success if no errors found, error if any error occurured during the first pass
 */
int first_pass(const char *filename, SymbolTable *symbol_table, MemoryImage *memory) {
    FILE *file;     /* file pointer to store the .am source file */
    char line[MAX_LINE_LENGTH];     /* buffer for current line being processed */   
    char label[MAX_LABEL_LENGTH + 1];      
    char directive[MAX_LABEL_LENGTH + 1];
    char operation[MAX_LABEL_LENGTH + 1];       
    char params[MAX_LINE_LENGTH];
    char operands[MAX_LINE_LENGTH];
    char *line_after_label;
    int line_num = 0;
    int error_found = FALSE;
    LineType line_type;
    char *label_ptr = NULL; /* label pointer for parse_directive and parse_instruction to check for label existence */

    file = fopen(filename, "r");
    /* make sure the file was opened successfully (error detection)*/
    if (!file) {
        print_error(0, ERR_FILE_OPEN, filename);
        return ERROR;
    }

    memory->IC = INITIAL_IC;
    memory->DC = INITIAL_DC;

    /* read and process the file line by line until EOF or error */
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        line_num++;

        /* make sure current line is not too long (error detection) */
        if (strlen(line) >= MAX_LINE_LENGTH - 1 && line[strlen(line) - 1] != '\n') {
            print_error(line_num, ERR_LINE_TOO_LONG, NULL);
            error_found = TRUE;
            continue;
        }

        line_type = parse_line_type(line);

        if (line_type == LINE_EMPTY) {
            continue;
        }

        line_after_label = parse_label(line, label, MAX_LABEL_LENGTH + 1);

        /* make sure that if a label exists its valid (error detection) */
        if (label[0] != '\0' && !is_valid_label(label)) {
            print_error(line_num, ERR_INVALID_LABEL, label);
            error_found = TRUE;
            continue;
        }

        /* update label pointer to note a valid label exists */
        label_ptr = NULL; /* reset last line's changes */
        if (label[0] != '\0') {
            label_ptr = label;
        }

        /* route line to appropriate handler based on its type */
        if (line_type == LINE_DIRECTIVE) {
            /* handle directive line */
            if (parse_directive(line_after_label, directive, params, MAX_LINE_LENGTH)) {
                if (process_directive(directive, params, label_ptr, line_num, symbol_table, memory) == ERROR) {
                    error_found = TRUE;
                }
            } else {
                /* parse_directive failed to process the directive line (error detection) */
                print_error(line_num, ERR_INVALID_DIRECTIVE, NULL);
                error_found = TRUE;
            }
        }
        else if (line_type == LINE_INSTRUCTION) {
            /* handle instruction line */
            if (parse_instruction(line_after_label, operation, operands, MAX_LINE_LENGTH)) {
                if (process_instruction(operation, operands, label_ptr, line_num, symbol_table, memory) == ERROR) {
                    error_found = TRUE;
                }
            } else {
                /* parse_instruction failed to process the instruction line (error detection) */
                print_error(line_num, ERR_INVALID_INSTRUCTION, NULL);
                error_found = TRUE;
            }
        }
        else {
            /* unrecognised line type */
            print_error(line_num, ERR_NONE, "unrecognised line type");
            error_found = TRUE;
        }
    }

    fclose(file);

     /* if any errors were found during the first pass return ERROR to stop the assembler process*/
    if (error_found) {
        return ERROR;
    }

    memory->ICF = memory->IC;
    memory->DCF = memory->DC;

    /* once the final length of the code image is known shift all data symbol addresses by ICF to reflect their actual position after the code section */
    symbol_table_update_data_symbols(symbol_table, memory->ICF);

    return SUCCESS;
}