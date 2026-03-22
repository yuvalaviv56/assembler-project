/*
 * errors.h
 * Error handling and reporting functions
 */

#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>

/* Error types */
typedef enum {
    ERR_NONE,
    ERR_FILE_OPEN,
    ERR_LINE_TOO_LONG,
    ERR_INVALID_LABEL,
    ERR_DUPLICATE_LABEL,
    ERR_UNDEFINED_LABEL,
    ERR_INVALID_INSTRUCTION,
    ERR_INVALID_DIRECTIVE,
    ERR_INVALID_OPERAND,
    ERR_ILLEGAL_ADDRESSING,
    ERR_OPERAND_COUNT,
    ERR_INVALID_REGISTER,
    ERR_INVALID_NUMBER,
    ERR_MACRO_NAME,
    ERR_MACRO_UNCLOSED,
    ERR_MEMORY_OVERFLOW,
    ERR_EXTERNAL_ENTRY_CONFLICT,
    ERR_MISSING_COMMA,
    ERR_EXTRA_COMMA,
    ERR_MISSING_QUOTE,
    ERR_EMPTY_DIRECTIVE
} ErrorType;

/*
 * Print error message with line number
 * Parameters:
 *   line_num - Line number where error occurred (0 if not applicable)
 *   type - Error type
 *   message - Additional error message (can be NULL)
 */
void print_error(int line_num, ErrorType type, const char *message);

/*
 * Print warning message with line number
 * Parameters:
 *   line_num - Line number where warning occurred
 *   message - Warning message
 */
void print_warning(int line_num, const char *message);

/*
 * Get error message string for error type
 * Parameters:
 *   type - Error type
 * Returns: Error message string
 */
const char* get_error_message(ErrorType type);

#endif /* ERRORS_H */