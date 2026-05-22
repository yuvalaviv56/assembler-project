/*
 * parser.h
 * Declaration of assembly line parsing functions
 */

#ifndef PARSER_H
#define PARSER_H

#include "structures.h"

/*
 * Determines the type of an assembly source line
 * Input: line string
 * Output: one of the options: LINE_EMPTY, LINE_DIRECTIVE, LINE_INSTRUCTION, or LINE_ERROR
 */
LineType parse_line_type(const char *line);

/*
 * Extracts the label from current line if one exists
 * Input: line string, buffer to store label, max label length
 * Output: pointer to the point in the line directly after the label, or original line if no label exists
 */
char* parse_label(char *line, char *label, int max_len);

/*
 * Extracts the directive name and parameters from a directive line
 * Input: line string, buffer for directive name, buffer for parameters, max length for either buffer
 * Output: TRUE on success, FALSE if any input is NULL
 */
int parse_directive(char *line, char *directive, char *params, int max_len);

/*
 * Extracts the operation name and operands from an instruction line
 * Input: line string, buffer for operation name, buffer for operands, max length for either buffer
 * Output: TRUE on success, FALSE if any input is NULL
 */
int parse_instruction(char *line, char *operation, char *operands, int max_len);

/*
 * Splits an operands string into source and destination operands
 * Input: operands string, buffer for source operand, buffer for destination operand, max length for either buffer
 * Output: number of operands found (0, 1, or 2)
 */
int parse_operands(char *operands_str, char *source, char *dest, int max_len);

/*
 * Identifies the addressing mode of an operand string
 * Input: operand string
 * Output: one of the options: MODE_IMMEDIATE, MODE_DIRECT, MODE_RELATIVE, or MODE_REGISTER
 */
AddressingMode identify_addressing_mode(const char *operand);

/*
 * Breaks down a single operand string and fills an Operand struct with the corresponding data
 * Input: operand string, pointer to Operand struct to fill
 * Output: TRUE on success, FALSE if input is NULL or parsing fails
 */
int parse_operand(const char *operand_str, Operand *operand);

/*
 * Make sure a string is a valid assembly operation name (error detection)
 * Input: operation name string
 * Output: TRUE if valid, FALSE otherwise
 */
int is_valid_operation(const char *operation);

/*
 * Checks if a string is a valid directive name (error detection)
 * Input: directive name string
 * Output: TRUE if valid, FALSE otherwise
 */
int is_valid_directive(const char *directive);

/*
 * Converts a string representation to a decimal integer
 * Input: string to parse, pointer to int to store result
 * Output: TRUE if valid integer, FALSE if invalid or NULL input
 */
int parse_integer(const char *str, int *value);

/*
 * Validates a register operand and extracts its number
 * Input: register string, pointer to int to store register number
 * Output: TRUE if valid register (r0-r7), FALSE otherwise
 */
int parse_register(const char *str, int *reg_num);

#endif /* PARSER_H */