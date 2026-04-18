/*
 * first_pass.c
 * First pass of the assembler - builds symbol table and basic encoding
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

/* Operation table - maps operation names to opcodes and funct codes */
typedef struct {
    const char *name;
    int opcode;
    int funct;
    int num_operands;
} OperationInfo;

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

/* Get operation information */
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

/* Calculate instruction length in memory words */
int calculate_instruction_length(const char *operation, const char *source, const char *dest) {
    int length = 1; /* First word always present */
    int num_operands;
    AddressingMode src_mode, dest_mode;
    
    /* Get number of operands */
    if (!get_operation_info(operation, NULL, NULL, &num_operands)) {
        return 1; /* Default if operation not found */
    }
    
    /* No operands - just first word */
    if (num_operands == 0) {
        return 1;
    }
    
    /* One operand (destination only) */
    if (num_operands == 1 && dest != NULL) {
        dest_mode = identify_addressing_mode(dest);
        /* Register mode doesn't need additional word in first pass (combined in first word) */
        if (dest_mode != MODE_REGISTER) {
            length++;
        }
        return length;
    }
    
    /* Two operands */
    if (num_operands == 2 && source != NULL && dest != NULL) {
        src_mode = identify_addressing_mode(source);
        dest_mode = identify_addressing_mode(dest);
        
        /* Special case: both registers can share one additional word */
        if (src_mode == MODE_REGISTER && dest_mode == MODE_REGISTER) {
            length++; /* One word for both registers */
        } else {
            /* Each non-register operand needs its own word */
            if (src_mode != MODE_REGISTER) length++;
            if (dest_mode != MODE_REGISTER) length++;
        }
    }
    
    return length;
}

/* Encode first word of instruction */
static void encode_first_word(MemoryImage *memory, const char *operation, 
                              const char *source, const char *dest) {
    int opcode, funct, num_operands;
    unsigned int word = 0;
    AddressingMode src_mode = MODE_IMMEDIATE, dest_mode = MODE_IMMEDIATE;
    
    /* Get operation info */
    if (!get_operation_info(operation, &opcode, &funct, &num_operands)) {
        return;
    }
    
    /* Encode opcode (bits 11-14) */
    word |= ((unsigned int)opcode & 0xF) << 11;
    
    /* Encode funct (bits 7-10) */
    word |= ((unsigned int)funct & 0xF) << 7;
    
    /* Encode addressing modes */
    if (num_operands >= 1 && dest != NULL) {
        dest_mode = identify_addressing_mode(dest);
        word |= ((unsigned int)dest_mode & 0x3) << 3;
    }
    
    if (num_operands == 2 && source != NULL) {
        src_mode = identify_addressing_mode(source);
        word |= ((unsigned int)src_mode & 0x3) << 5;
    }
    
    /* Store in code image */
    memory->code[memory->IC].word = word & 0xFFF;
    memory->code[memory->IC].are = ARE_ABSOLUTE;
}

/* Process .data directive */
int process_data_directive(const char *params, int line_num, MemoryImage *memory) {
    char params_copy[MAX_LINE_LENGTH];
    char *token;
    int value;
    
    if (params == NULL || *params == '\0') {
        print_error(line_num, ERR_EMPTY_DIRECTIVE, NULL);
        return ERROR;
    }
    
    safe_strcpy(params_copy, params, MAX_LINE_LENGTH);
    
    /* Parse comma-separated integers */
    token = strtok(params_copy, ",");
    while (token != NULL) {
        trim(token);
        
        if (!parse_integer(token, &value)) {
            print_error(line_num, ERR_INVALID_NUMBER, token);
            return ERROR;
        }
        
        /* Store in data image */
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

/* Process .string directive */
int process_string_directive(const char *params, int line_num, MemoryImage *memory) {
    const char *p;
    int in_quotes = FALSE;
    
    if (params == NULL || *params == '\0') {
        print_error(line_num, ERR_EMPTY_DIRECTIVE, NULL);
        return ERROR;
    }
    
    p = skip_whitespace(params);
    
    /* Must start with quote */
    if (*p != STRING_QUOTE) {
        print_error(line_num, ERR_MISSING_QUOTE, NULL);
        return ERROR;
    }
    
    p++; /* Skip opening quote */
    in_quotes = TRUE;
    
    /* Encode each character */
    while (*p != '\0' && *p != '\n') {
        if (*p == STRING_QUOTE) {
            in_quotes = FALSE;
            break;
        }
        
        if (memory->DC >= MEMORY_SIZE) {
            print_error(line_num, ERR_MEMORY_OVERFLOW, NULL);
            return ERROR;
        }
        
        /* Store ASCII value */
        memory->data[memory->DC].word = (unsigned int)(*p) & 0xFFF;
        memory->data[memory->DC].are = ARE_ABSOLUTE;
        memory->DC++;
        
        p++;
    }
    
    if (in_quotes) {
        print_error(line_num, ERR_MISSING_QUOTE, NULL);
        return ERROR;
    }
    
    /* Add null terminator */
    if (memory->DC >= MEMORY_SIZE) {
        print_error(line_num, ERR_MEMORY_OVERFLOW, NULL);
        return ERROR;
    }
    
    memory->data[memory->DC].word = 0;
    memory->data[memory->DC].are = ARE_ABSOLUTE;
    memory->DC++;
    
    return SUCCESS;
}

/* Process .entry directive (mark for second pass) */
int process_entry_directive(const char *params, int line_num) {
    /* Entry processing is done in second pass */
    /* Just validate that parameter exists */
    if (params == NULL || *params == '\0') {
        print_error(line_num, ERR_EMPTY_DIRECTIVE, NULL);
        return ERROR;
    }
    
    /* Validation will happen in second pass */
    return SUCCESS;
}

/* Process .extern directive */
int process_extern_directive(const char *params, int line_num, SymbolTable *symbol_table) {
    char symbol_name[MAX_LABEL_LENGTH + 1];
    
    if (params == NULL || *params == '\0') {
        print_error(line_num, ERR_EMPTY_DIRECTIVE, NULL);
        return ERROR;
    }
    
    /* Extract symbol name */
    safe_strcpy(symbol_name, params, MAX_LABEL_LENGTH + 1);
    trim(symbol_name);
    
    /* Validate symbol name */
    if (!is_valid_label(symbol_name)) {
        print_error(line_num, ERR_INVALID_LABEL, symbol_name);
        return ERROR;
    }
    
    /* Add to symbol table with EXTERNAL attribute */
    if (!symbol_table_add(symbol_table, symbol_name, 0, ATTR_EXTERNAL)) {
        print_error(line_num, ERR_DUPLICATE_LABEL, symbol_name);
        return ERROR;
    }
    
    return SUCCESS;
}

/* Process directive line */
int process_directive(const char *directive, const char *params, const char *label,
                      int line_num, SymbolTable *symbol_table, MemoryImage *memory) {
    
    /* If there's a label, add it to symbol table */
    if (label != NULL && *label != '\0') {
        /* For .entry and .extern, labels are not allowed */
        if (strcmp(directive, DIR_ENTRY) == 0 || strcmp(directive, DIR_EXTERN) == 0) {
            print_error(line_num, ERR_NONE, "Label not allowed with .entry or .extern");
            return ERROR;
        }
        
        /* Add label with DATA attribute and current DC */
        if (!symbol_table_add(symbol_table, label, memory->DC, ATTR_DATA)) {
            print_error(line_num, ERR_DUPLICATE_LABEL, label);
            return ERROR;
        }
    }
    
    /* Process specific directive */
    if (strcmp(directive, DIR_DATA) == 0) {
        return process_data_directive(params, line_num, memory);
    }
    
    if (strcmp(directive, DIR_STRING) == 0) {
        return process_string_directive(params, line_num, memory);
    }
    
    if (strcmp(directive, DIR_ENTRY) == 0) {
        return process_entry_directive(params, line_num);
    }
    
    if (strcmp(directive, DIR_EXTERN) == 0) {
        return process_extern_directive(params, line_num, symbol_table);
    }
    
    print_error(line_num, ERR_INVALID_DIRECTIVE, directive);
    return ERROR;
}

/* Process instruction line */
int process_instruction(const char *operation, const char *operands, const char *label,
                        int line_num, SymbolTable *symbol_table, MemoryImage *memory) {
    char source[MAX_LINE_LENGTH] = "";
    char dest[MAX_LINE_LENGTH] = "";
    char operands_copy[MAX_LINE_LENGTH];
    int num_operands_parsed;
    int num_operands_expected;
    int instruction_length;
    
    /* If there's a label, add it to symbol table with CODE attribute */
    if (label != NULL && *label != '\0') {
        if (!symbol_table_add(symbol_table, label, memory->IC, ATTR_CODE)) {
            print_error(line_num, ERR_DUPLICATE_LABEL, label);
            return ERROR;
        }
    }
    
    /* Validate operation */
    if (!get_operation_info(operation, NULL, NULL, &num_operands_expected)) {
        print_error(line_num, ERR_INVALID_INSTRUCTION, operation);
        return ERROR;
    }
    
    /* Parse operands */
    safe_strcpy(operands_copy, operands, MAX_LINE_LENGTH);
    num_operands_parsed = parse_operands(operands_copy, source, dest, MAX_LINE_LENGTH);
    
    /* Validate operand count */
    if (num_operands_parsed != num_operands_expected) {
        print_error(line_num, ERR_OPERAND_COUNT, operation);
        return ERROR;
    }
    
    /* Calculate instruction length */
    instruction_length = calculate_instruction_length(operation, 
                                                      num_operands_expected == 2 ? source : NULL,
                                                      num_operands_expected >= 1 ? dest : NULL);
    
    /* Check memory overflow */
    if (memory->IC + instruction_length > MEMORY_SIZE) {
        print_error(line_num, ERR_MEMORY_OVERFLOW, NULL);
        return ERROR;
    }
    
    /* Encode first word */
    encode_first_word(memory, operation, 
                     num_operands_expected == 2 ? source : NULL,
                     num_operands_expected >= 1 ? dest : NULL);
    
    /* Advance IC by instruction length */
    memory->IC += instruction_length;
    
    return SUCCESS;
}

/* Main first pass function */
int first_pass(const char *filename, SymbolTable *symbol_table, MemoryImage *memory) {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    char label[MAX_LABEL_LENGTH + 1];
    char directive[MAX_LABEL_LENGTH + 1];
    char operation[MAX_LABEL_LENGTH + 1];
    char params[MAX_LINE_LENGTH];
    char operands[MAX_LINE_LENGTH];
    char *line_after_label;
    int line_num = 0;
    int error_found = FALSE;
    LineType line_type;
    
    /* Open file */
    file = fopen(filename, "r");
    if (!file) {
        print_error(0, ERR_FILE_OPEN, filename);
        return ERROR;
    }
    
    /* Initialize memory */
    memory->IC = INITIAL_IC;
    memory->DC = INITIAL_DC;
    
    /* Process each line */
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        line_num++;
        
        /* Check line length */
        if (strlen(line) >= MAX_LINE_LENGTH - 1 && line[strlen(line) - 1] != '\n') {
            print_error(line_num, ERR_LINE_TOO_LONG, NULL);
            error_found = TRUE;
            continue;
        }
        
        /* Determine line type */
        line_type = parse_line_type(line);
        
        if (line_type == LINE_EMPTY) {
            continue; /* Skip empty lines and comments */
        }
        
        /* Extract label if present */
        line_after_label = parse_label(line, label, MAX_LABEL_LENGTH + 1);
        
        /* Validate label if present */
        if (label[0] != '\0' && !is_valid_label(label)) {
            print_error(line_num, ERR_INVALID_LABEL, label);
            error_found = TRUE;
            continue;
        }
        
        /* Process based on line type */
        if (line_type == LINE_DIRECTIVE) {
            if (parse_directive(line_after_label, directive, params, MAX_LINE_LENGTH)) {
                if (process_directive(directive, params, label[0] != '\0' ? label : NULL,
                                     line_num, symbol_table, memory) == ERROR) {
                    error_found = TRUE;
                }
            } else {
                print_error(line_num, ERR_INVALID_DIRECTIVE, NULL);
                error_found = TRUE;
            }
        }
        else if (line_type == LINE_INSTRUCTION) {
            if (parse_instruction(line_after_label, operation, operands, MAX_LINE_LENGTH)) {
                if (process_instruction(operation, operands, label[0] != '\0' ? label : NULL,
                                       line_num, symbol_table, memory) == ERROR) {
                    error_found = TRUE;
                }
            } else {
                print_error(line_num, ERR_INVALID_INSTRUCTION, NULL);
                error_found = TRUE;
            }
        }
        else {
            print_error(line_num, ERR_NONE, "Unable to parse line");
            error_found = TRUE;
        }
    }
    
    fclose(file);
    
    /* If errors found, stop here */
    if (error_found) {
        return ERROR;
    }
    
    /* Save final IC and DC values */
    memory->ICF = memory->IC;
    memory->DCF = memory->DC;
    
    /* Update data symbols: add ICF to their values */
    symbol_table_update_data_symbols(symbol_table, memory->ICF);
    
    return SUCCESS;
}