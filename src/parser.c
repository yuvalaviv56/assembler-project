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

/* Parse line type */
LineType parse_line_type(const char *line) {
    const char *p;
    char first_word[MAX_LABEL_LENGTH + 1];
    
    if (line == NULL) {
        return LINE_ERROR;
    }
    
    /* Check if empty or comment */
    if (is_empty_or_comment(line)) {
        return LINE_EMPTY;
    }
    
    /* Skip label if present */
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
    
    return LINE_ERROR;
}

/* Extract label from line */
char* parse_label(char *line, char *label, int max_len) {
    char *colon_pos;
    int label_len;
    
    if (line == NULL || label == NULL) {
        return line;
    }
    
    /* Initialize label as empty */
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
    
    /* Check if label is too long */
    if (label_len >= max_len) {
        return line;
    }
    
    /* Extract label */
    strncpy(label, line, label_len);
    label[label_len] = '\0';
    
    /* Trim whitespace from label */
    trim(label);
    
    /* Return pointer to rest of line after colon */
    return skip_whitespace(colon_pos + 1);
}

/* Parse directive line */
int parse_directive(char *line, char *directive, char *params, int max_len) {
    char *p;
    
    if (line == NULL || directive == NULL || params == NULL) {
        return FALSE;
    }
    
    /* Initialize outputs */
    directive[0] = '\0';
    params[0] = '\0';
    
    /* Skip whitespace */
    p = skip_whitespace(line);
    
    /* Extract directive */
    p = extract_word(p, directive, max_len);
    
    /* Get parameters (rest of line) */
    p = skip_whitespace(p);
    safe_strcpy(params, p, max_len);
    trim(params);
    
    return TRUE;
}

/* Parse instruction line */
int parse_instruction(char *line, char *operation, char *operands, int max_len) {
    char *p;
    
    if (line == NULL || operation == NULL || operands == NULL) {
        return FALSE;
    }
    
    /* Initialize outputs */
    operation[0] = '\0';
    operands[0] = '\0';
    
    /* Skip whitespace */
    p = skip_whitespace(line);
    
    /* Extract operation */
    p = extract_word(p, operation, max_len);
    
    /* Get operands (rest of line) */
    p = skip_whitespace(p);
    safe_strcpy(operands, p, max_len);
    trim(operands);
    
    return TRUE;
}

/* Parse operands string */
int parse_operands(char *operands_str, char *source, char *dest, int max_len) {
    char *comma;
    
    if (operands_str == NULL) {
        return 0;
    }
    
    /* Initialize outputs */
    if (source != NULL) source[0] = '\0';
    if (dest != NULL) dest[0] = '\0';
    
    /* Trim whitespace */
    trim(operands_str);
    
    /* If empty, no operands */
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
        *comma = '\0';
        safe_strcpy(source, operands_str, max_len);
        trim(source);
        *comma = OPERAND_SEPARATOR; /* Restore comma */
    }
    
    if (dest != NULL) {
        /* Extract destination (after comma) */
        safe_strcpy(dest, comma + 1, max_len);
        trim(dest);
    }
    
    return 2;
}

/* Identify addressing mode */
AddressingMode identify_addressing_mode(const char *operand) {
    int reg_num;
    
    if (operand == NULL || *operand == '\0') {
        return MODE_IMMEDIATE; /* Default/error case */
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

/* Parse operand */
int parse_operand(const char *operand_str, Operand *operand) {
    int value;
    
    if (operand_str == NULL || operand == NULL) {
        return FALSE;
    }
    
    /* Initialize */
    operand->mode = identify_addressing_mode(operand_str);
    operand->value = 0;
    operand->symbol[0] = '\0';
    
    switch (operand->mode) {
        case MODE_IMMEDIATE:
            /* Parse immediate value (skip #) */
            if (!parse_integer(operand_str + 1, &value)) {
                return FALSE;
            }
            operand->value = value;
            break;
            
        case MODE_REGISTER:
            /* Parse register number */
            if (!parse_register(operand_str, &value)) {
                return FALSE;
            }
            operand->value = value;
            break;
            
        case MODE_RELATIVE:
            /* Store symbol (skip %) */
            safe_strcpy(operand->symbol, operand_str + 1, MAX_LABEL_LENGTH + 1);
            break;
            
        case MODE_DIRECT:
            /* Store symbol */
            safe_strcpy(operand->symbol, operand_str, MAX_LABEL_LENGTH + 1);
            break;
    }
    
    return TRUE;
}

/* Check if valid operation */
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

/* Check if valid directive */
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

/* Parse integer */
int parse_integer(const char *str, int *value) {
    char *endptr;
    long result;
    
    if (str == NULL || value == NULL || *str == '\0') {
        return FALSE;
    }
    
    /* Skip whitespace */
    while (is_whitespace(*str)) {
        str++;
    }
    
    /* Parse integer */
    result = strtol(str, &endptr, 10);
    
    /* Check if entire string was consumed (except trailing whitespace) */
    while (is_whitespace(*endptr)) {
        endptr++;
    }
    
    if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r') {
        return FALSE; /* Invalid characters */
    }
    
    *value = (int)result;
    return TRUE;
}

/* Parse register */
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