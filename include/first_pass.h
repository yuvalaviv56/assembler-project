/*
 * first_pass.h
 * Function declarations for the first pass of the assembler
 */


#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "structures.h"
#include "symbol_table.h"

/*
 * The equvilant to the "main" function of the first pass - reads the .am file, builds the symbol table and encodes instruction first words into the code image
 * Input: path to .am file, symbol table, memory image
 * Output: SUCCESS if no errors found, ERROR if any error occurured during the first pass
 */
int first_pass(const char *filename, SymbolTable *symbol_table, MemoryImage *memory);

/*
 * Routes a directive line to the appropriate handler based on its type
 * Input: directive name, parameters, label, line number (for error reporting), symbol table, memory image
 * Output: SUCCESS if directive processed properly, ERROR if invalid input or processing failed
 */
int process_directive(const char *directive, const char *params, const char *label,
                      int line_num, SymbolTable *symbol_table, MemoryImage *memory);

/*
 * handles an instruction line by validating operands, encoding the first word and advancing the IC
 * Input: operation name, operands string, label, line number (for error reporting), symbol table, memory image
 * Output: SUCCESS if instruction processed properly, ERROR if invalid operands or memory overflow
 */
int process_instruction(const char *operation, const char *operands, const char *label,
                        int line_num, SymbolTable *symbol_table, MemoryImage *memory);

/*
 * handles a .data directive by storing the accompanying int values in the data image
 * Input: parameter string (comma-separated integers), line number (for error reporting), memory image
 * Output: SUCCESS if all values stored, ERROR if invalid input or memory overflow
 */
int process_data_directive(const char *params, int line_num, MemoryImage *memory);

/*
 * handles a .string directive by storing the ASCII codes of each character in the data image
 * Input: parameter string (quoted string), line number (for error reporting), memory image
 * Output: SUCCESS if string stored, ERROR if missing quotes or memory overflow
 */
int process_string_directive(const char *params, int line_num, MemoryImage *memory);

/*
 * Validates a .entry directive - actual handling is done during the second pass
 * Input: parameter string (label name), line number (for error reporting)
 * Output: SUCCESS if parameter exists, ERROR if empty
 */
int process_entry_directive(const char *params, int line_num);

/*
 * handles a .extern directive by adding the external symbol to the symbol table
 * Input: parameter string (label name), line number (for error reporting), symbol table
 * Output: SUCCESS if symbol added, ERROR if invalid name or duplicate
 */
int process_extern_directive(const char *params, int line_num, SymbolTable *symbol_table);

/*
 * Looks up an operation by name and retrieves the details of its properties
 * Input: operation name string, pointers to store the opcode, funct, and operand count values (any can be NULL)
 * Output: TRUE if operation found, FALSE otherwise
 */
int get_operation_info(const char *operation, int *opcode, int *funct, int *num_operands);

/*
 * Calculates the number of memory words an instruction occupies
 * Input: operation name, source operand string, destination operand string
 * Output: number of words (1 for no operands, and up to 3 for two operands)
 */
int calculate_instruction_length(const char *operation, const char *source, const char *dest);

#endif /* FIRST_PASS_H */