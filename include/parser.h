/*
 * parser.h
 * Assembly line parsing functions
 */

#ifndef PARSER_H
#define PARSER_H

#include "structures.h"

/*
 * Parse a line and determine its type
 * Parameters:
 *   line - Line to parse
 * Returns: LINE_EMPTY, LINE_DIRECTIVE, LINE_INSTRUCTION, or LINE_ERROR
 */
LineType parse_line_type(const char *line);

/*
 * Extract label from line (if exists)
 * Parameters:
 *   line - Line to parse
 *   label - Buffer to store label (output)
 *   max_len - Maximum label length
 * Returns: Pointer to rest of line after label, or original line if no label
 */
char* parse_label(char *line, char *label, int max_len);

/*
 * Parse a directive line (.data, .string, .entry, .extern)
 * Parameters:
 *   line - Line to parse (after label if any)
 *   directive - Buffer to store directive name (output)
 *   params - Buffer to store parameters (output)
 *   max_len - Maximum buffer length
 * Returns: TRUE if successfully parsed, FALSE otherwise
 */
int parse_directive(char *line, char *directive, char *params, int max_len);

/*
 * Parse an instruction line (operation + operands)
 * Parameters:
 *   line - Line to parse (after label if any)
 *   operation - Buffer to store operation name (output)
 *   operands - Buffer to store operands string (output)
 *   max_len - Maximum buffer length
 * Returns: TRUE if successfully parsed, FALSE otherwise
 */
int parse_instruction(char *line, char *operation, char *operands, int max_len);

/*
 * Parse operands string and extract individual operands
 * Parameters:
 *   operands_str - String containing operands (e.g., "#5, r1")
 *   source - Buffer for source operand (output, can be NULL if none)
 *   dest - Buffer for destination operand (output, can be NULL if none)
 *   max_len - Maximum buffer length
 * Returns: Number of operands found (0, 1, or 2)
 */
int parse_operands(char *operands_str, char *source, char *dest, int max_len);

/*
 * Identify addressing mode of an operand
 * Parameters:
 *   operand - Operand string (e.g., "#5", "r1", "LABEL", "%LOOP")
 * Returns: AddressingMode (MODE_IMMEDIATE, MODE_DIRECT, MODE_RELATIVE, MODE_REGISTER)
 */
AddressingMode identify_addressing_mode(const char *operand);

/*
 * Parse an operand and extract its components
 * Parameters:
 *   operand_str - Operand string
 *   operand - Operand structure to fill (output)
 * Returns: TRUE if successfully parsed, FALSE otherwise
 */
int parse_operand(const char *operand_str, Operand *operand);

/*
 * Check if string is a valid operation name
 * Parameters:
 *   operation - Operation name to check
 * Returns: TRUE if valid operation, FALSE otherwise
 */
int is_valid_operation(const char *operation);

/*
 * Check if string is a valid directive
 * Parameters:
 *   directive - Directive name to check
 * Returns: TRUE if valid directive, FALSE otherwise
 */
int is_valid_directive(const char *directive);

/*
 * Parse integer from string (handles positive/negative)
 * Parameters:
 *   str - String to parse
 *   value - Pointer to store result (output)
 * Returns: TRUE if valid integer, FALSE otherwise
 */
int parse_integer(const char *str, int *value);

/*
 * Parse register name (r0-r7)
 * Parameters:
 *   str - String to parse (e.g., "r1")
 *   reg_num - Pointer to store register number (output)
 * Returns: TRUE if valid register, FALSE otherwise
 */
int parse_register(const char *str, int *reg_num);

#endif /* PARSER_H */