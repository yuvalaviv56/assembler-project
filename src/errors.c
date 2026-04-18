/*
 * errors.c
 * Error handling and reporting
 */

#include <stdio.h>
#include "errors.h"
#include "globals.h"

/* Get error message for error type */
const char* get_error_message(ErrorType type) {
    switch (type) {
        case ERR_NONE:
            return "No error";
        case ERR_FILE_OPEN:
            return "Cannot open file";
        case ERR_LINE_TOO_LONG:
            return "Line exceeds maximum length (80 characters)";
        case ERR_INVALID_LABEL:
            return "Invalid label name";
        case ERR_DUPLICATE_LABEL:
            return "Label already defined";
        case ERR_UNDEFINED_LABEL:
            return "Undefined label";
        case ERR_INVALID_INSTRUCTION:
            return "Invalid instruction";
        case ERR_INVALID_DIRECTIVE:
            return "Invalid directive";
        case ERR_INVALID_OPERAND:
            return "Invalid operand";
        case ERR_ILLEGAL_ADDRESSING:
            return "Illegal addressing mode for this instruction";
        case ERR_OPERAND_COUNT:
            return "Wrong number of operands";
        case ERR_INVALID_REGISTER:
            return "Invalid register";
        case ERR_INVALID_NUMBER:
            return "Invalid number format";
        case ERR_MACRO_NAME:
            return "Invalid macro name (cannot be instruction/directive/register)";
        case ERR_MACRO_UNCLOSED:
            return "Macro definition not closed (missing 'mcroend')";
        case ERR_MEMORY_OVERFLOW:
            return "Memory overflow - program too large";
        case ERR_EXTERNAL_ENTRY_CONFLICT:
            return "Symbol cannot be both external and entry";
        case ERR_MISSING_COMMA:
            return "Missing comma between operands";
        case ERR_EXTRA_COMMA:
            return "Extraneous comma";
        case ERR_MISSING_QUOTE:
            return "Missing closing quote in string";
        case ERR_EMPTY_DIRECTIVE:
            return "Directive has no parameters";
        default:
            return "Unknown error";
    }
}

/* Print error with line number */
void print_error(int line_num, ErrorType type, const char *message) {
    if (line_num > 0) {
        fprintf(stderr, "Error (line %d): %s", line_num, get_error_message(type));
    } else {
        fprintf(stderr, "Error: %s", get_error_message(type));
    }
    
    if (message != NULL && *message != '\0') {
        fprintf(stderr, " - %s", message);
    }
    
    fprintf(stderr, "\n");
}

/* Print warning */
void print_warning(int line_num, const char *message) {
    if (line_num > 0) {
        fprintf(stderr, "Warning (line %d): %s\n", line_num, message);
    } else {
        fprintf(stderr, "Warning: %s\n", message);
    }
}