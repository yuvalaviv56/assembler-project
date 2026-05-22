/*
 * parser.c
 * Implementation of assembly line parsing functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "string_utils.h"
#include "constants.h"
#include "globals.h"

/*
 * Determines the type of an assembly source line
 * Input: line string
 * Output: one of the options: LINE_EMPTY, LINE_DIRECTIVE, LINE_INSTRUCTION, or LINE_ERROR
 */
LineType parse_line_type(const char *line) {
    const char *p;      /* pointer for going through the line's content */
    char first_word[MAX_LABEL_LENGTH + 1]; /* extract first word after label (if any) */
    
    /* Make sure current line has content (error detection) */
    if (line == NULL) {
        return LINE_ERROR;
    }
    
    /* Check if current line is empty or comment (no need for parsing)*/
    if (is_empty_or_comment(line)) {
        return LINE_EMPTY;
    }
    
    /* Skip label if there is one */
    p = strchr(line, LABEL_END);
    if (p != NULL) {
        p++; /* Skip the ':' */
        p = skip_whitespace(p);
    } else {
        p = skip_whitespace(line);
    }
    
    /* Extract first word */
    extract_word((char*)p, first_word, MAX_LABEL_LENGTH + 1);
    
    /* Check if directive */
    if (first_word[0] == DIRECTIVE_PREFIX) {
        return LINE_DIRECTIVE;
    }
    
    /* Check if instruction */
    if (is_valid_operation(first_word)) {
        return LINE_INSTRUCTION;
    }
    
    /* return error in case no line type was assigned successfully */
    return LINE_ERROR;
}

/*
 * Extracts the label from current line if one exists
 * Input: line string, buffer to store label, max label length
 * Output: pointer to the point in the line directly after the label, or original line if no label exists
 */
char* parse_label(char *line, char *label, int max_len) {
    char *colon_pos;    /* pointer to the ':' character if exists */
    int label_len;      /* length of the label string */
    
    /*Make sure the current line has content and the label buffer is valid*/
    if (line == NULL || label == NULL) {
        return line;
    }
    
    /* Initialize label buffer as empty */
    label[0] = '\0';
    
    /* Skip leading whitespace */
    line = skip_whitespace(line);
    
    /* Find colon */
    colon_pos = strchr(line, LABEL_END);
    if (colon_pos == NULL) {
        return line; /* No label */
    }
    
    /* Calculate label length */
    label_len = colon_pos - line;
    
    /* Make sure label length is valid */
    if (label_len >= max_len) {
        return line;
    }
    
    /* Extract label to buffer */
    strncpy(label, line, label_len);
    label[label_len] = '\0';
    
    /* Trim whitespace from label */
    trim(label);
    
    /* Return pointer to rest of line after colon */
    return skip_whitespace(colon_pos + 1);
}

/*
 * Extracts the directive name and parameters from a directive line
 * Input: line string, buffer for directive name, buffer for parameters, max length for either buffer
 * Output: TRUE on success, FALSE if any input is NULL
 */
int parse_directive(char *line, char *directive, char *params, int max_len) {
    char *p; /* pointer for going through the line's content */
    
    if (line == NULL || directive == NULL || params == NULL) {
        return FALSE;
    }
    
    /* Initialize output buffers as an empty string */
    directive[0] = '\0';
    params[0] = '\0';
    
    /* Skip whitespace */
    p = skip_whitespace(line);
    
    /* Extract directive to directive buffer */
    p = extract_word(p, directive, max_len);
    
    /* Get parameters (rest of line) and store in params buffer */
    p = skip_whitespace(p);
    safe_strcpy(params, p, max_len);
    trim(params);
    
    return TRUE;
}

/*
 * Extracts the operation name and operands from an instruction line
 * Input: line string, buffer for operation name, buffer for operands, max length for either buffer
 * Output: TRUE on success, FALSE if any input is NULL
 */
int parse_instruction(char *line, char *operation, char *operands, int max_len) {
    char *p;    /* pointer for going through the line's content */
    
    if (line == NULL || operation == NULL || operands == NULL) {
        return FALSE;
    }
    
     /* Initialize output buffers as an empty string */
    operation[0] = '\0';
    operands[0] = '\0';
    
    /* Skip whitespace */
    p = skip_whitespace(line);
    
    /* Extract operation to operation buffer */
    p = extract_word(p, operation, max_len);
    
    /* Get operands (rest of line) and store in operands buffer */
    p = skip_whitespace(p);
    safe_strcpy(operands, p, max_len);
    trim(operands);
    
    return TRUE;
}

/*
 * Splits an operands string into source and destination operands
 * Input: operands string, buffer for source operand, buffer for destination operand, max length for either buffer
 * Output: number of operands found (0, 1, or 2)
 */
int parse_operands(char *operands_str, char *source, char *dest, int max_len) {
    char *comma;
    
    if (operands_str == NULL) {
        return 0;
    }
    
    /* make sure the buffers provided are valid and if so initialize them as empty strings */
    if (source != NULL) source[0] = '\0';
    if (dest != NULL) dest[0] = '\0';
    
    /* Trim whitespaces from operand's input string before processing */
    trim(operands_str);
    
    /* If input string is empty there are no operands */
    if (*operands_str == '\0') {
        return 0;
    }
    
    /* Find comma separator */
    comma = strchr(operands_str, OPERAND_SEPARATOR);
    
    if (comma == NULL) {
        /* Single operand (destination only) */
        if (dest != NULL) {
            safe_strcpy(dest, operands_str, max_len);
            trim(dest);
        }
        return 1;
    }
    
    /* Two operands */
    if (source != NULL) {
        /* Extract source (before comma) */
        *comma = '\0';      /* Temporarily split source string up to comma */
        safe_strcpy(source, operands_str, max_len);     /* copy source operand to source buffer */
        trim(source);
        *comma = OPERAND_SEPARATOR; /* Restore comma */
    }
    
    if (dest != NULL) {
        /* Extract destination (after comma) */
        safe_strcpy(dest, comma + 1, max_len); /* copy destination operand to destination buffer */
        trim(dest);
    }
    
    return 2;
}

/*
 * Identifies the addressing mode of an operand string
 * Input: operand string
 * Output: one of the options: MODE_IMMEDIATE, MODE_DIRECT, MODE_RELATIVE, or MODE_REGISTER
 */
AddressingMode identify_addressing_mode(const char *operand) {
    int reg_num;    /* required by parse_register to validate register operands */
    
    /* Default or error case */
    if (operand == NULL || *operand == '\0') {
        return MODE_IMMEDIATE;
    }
    
    /* Immediate: starts with # */
    if (operand[0] == IMMEDIATE_PREFIX) {
        return MODE_IMMEDIATE;
    }
    
    /* Relative: starts with % */
    if (operand[0] == RELATIVE_PREFIX) {
        return MODE_RELATIVE;
    }
    
    /* Register: r0-r7 */
    if (parse_register(operand, &reg_num)) {
        return MODE_REGISTER;
    }
    
    /* Direct: label */
    return MODE_DIRECT;
}

/*
 * Breaks down a single operand string and fills an Operand struct with the corresponding data
 * Input: operand string, pointer to Operand struct to fill
 * Output: TRUE on success, FALSE if input is NULL or parsing fails
 */
int parse_operand(const char *operand_str, Operand *operand) {
    int value;      /* numeric value (immediate or register number) */

    
    if (operand_str == NULL || operand == NULL) {
        return FALSE;
    }
    
    /* Initialize properties */
    operand->mode = identify_addressing_mode(operand_str);
    operand->value = 0;
    operand->symbol[0] = '\0';
    
    /*process operand based on addressing mode */
    switch (operand->mode) {
        case MODE_IMMEDIATE:
            /* immediate mode (skip '#' char and store the int value) */
            if (!parse_integer(operand_str + 1, &value)) {
                return FALSE;
            }
            operand->value = value;
            break;
            
        case MODE_REGISTER:
            /* register mode (extract and store the register number) */
            if (!parse_register(operand_str, &value)) {
                return FALSE;
            }
            operand->value = value;
            break;
            
        case MODE_RELATIVE:
            /* relative mode (skip '%' char and store the label name) */
            safe_strcpy(operand->symbol, operand_str + 1, MAX_LABEL_LENGTH + 1);
            break;
            
        case MODE_DIRECT:
            /* direct mode (store the label name as is) */
            safe_strcpy(operand->symbol, operand_str, MAX_LABEL_LENGTH + 1);
            break;
    }
    
    return TRUE;
}

/*
 * Make sure a string is a valid assembly operation name (error detection)
 * Input: operation name string
 * Output: TRUE if valid, FALSE otherwise
 */
int is_valid_operation(const char *operation) {
    static const char *operations[] = {
        "mov", "cmp", "add", "sub", "lea",
        "clr", "not", "inc", "dec",
        "jmp", "bne", "jsr", "red", "prn",
        "rts", "stop",
        NULL
    };
    
    int i;
    
    if (operation == NULL) {
        return FALSE;
    }
    
    for (i = 0; operations[i] != NULL; i++) {
        if (strcmp(operation, operations[i]) == 0) {
            return TRUE;
        }
    }
    
    return FALSE;
}

/*
 * Checks if a string is a valid directive name (error detection)
 * Input: directive name string
 * Output: TRUE if valid, FALSE otherwise
 */
int is_valid_directive(const char *directive) {
    if (directive == NULL) {
        return FALSE;
    }
    
    if (strcmp(directive, DIR_DATA) == 0 ||
        strcmp(directive, DIR_STRING) == 0 ||
        strcmp(directive, DIR_ENTRY) == 0 ||
        strcmp(directive, DIR_EXTERN) == 0) {
        return TRUE;
    }
    
    return FALSE;
}

/*
 * Converts a string representation to a decimal integer
 * Input: string to parse, pointer to int to store result
 * Output: TRUE if valid integer, FALSE if invalid or NULL input
 */
int parse_integer(const char *str, int *value) {
    char *endptr;   /* points to first char not consumed by strtol - used to make sure the string holds a valid int (error detection) */
    long result;    /* temporarily stores the converted long value before casting to int */
    
    if (str == NULL || value == NULL || *str == '\0') {
        return FALSE;
    }
    
    /* Skip whitespace */
    while (is_whitespace(*str)) {
        str++;
    }
    
    /* convert string to long integer in base 10 */
    result = strtol(str, &endptr, 10);
    
    /* Check if entire string was consumed (except trailing whitespace) */
    while (is_whitespace(*endptr)) {
        endptr++;
    }
    
    if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r') {
        return FALSE; /* Invalid characters in string (error detection) */
    }
    
    *value = (int)result;
    return TRUE;
}

/*
 * Validates a register operand and extracts its number
 * Input: register string, pointer to int to store register number
 * Output: TRUE if valid register (r0-r7), FALSE otherwise
 */
int parse_register(const char *str, int *reg_num) {
    if (str == NULL || reg_num == NULL) {
        return FALSE;
    }
    
    /* Must start with 'r' */
    if (str[0] != REGISTER_PREFIX) {
        return FALSE;
    }
    
    /* Must be followed by single digit 0-7 */
    if (str[1] < '0' || str[1] > '7') {
        return FALSE;
    }
    
    /* Must be only 2 characters (or followed by whitespace/newline) */
    if (str[2] != '\0' && str[2] != '\n' && str[2] != '\r' && !is_whitespace(str[2])) {
        return FALSE;
    }
    
    *reg_num = str[1] - '0';
    return TRUE;
}