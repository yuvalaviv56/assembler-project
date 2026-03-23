/*
 * first_pass.h
 * First pass of the assembler
 */

#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "structures.h"
#include "symbol_table.h"

/*
 * Perform first pass on assembly file
 * Builds symbol table and encodes basic instruction structure
 * 
 * Parameters:
 *   filename - Input .am filename
 *   symbol_table - Symbol table to populate
 *   memory - Memory image to populate
 * 
 * Returns: SUCCESS or ERROR
 */
int first_pass(const char *filename, SymbolTable *symbol_table, MemoryImage *memory);

/*
 * Process a directive line (.data, .string, .entry, .extern)
 * 
 * Parameters:
 *   directive - Directive name
 *   params - Directive parameters
 *   label - Label (if present, can be NULL)
 *   line_num - Line number (for error reporting)
 *   symbol_table - Symbol table
 *   memory - Memory image
 * 
 * Returns: SUCCESS or ERROR
 */
int process_directive(const char *directive, const char *params, const char *label,
                      int line_num, SymbolTable *symbol_table, MemoryImage *memory);

/*
 * Process an instruction line
 * 
 * Parameters:
 *   operation - Operation name
 *   operands - Operands string
 *   label - Label (if present, can be NULL)
 *   line_num - Line number (for error reporting)
 *   symbol_table - Symbol table
 *   memory - Memory image
 * 
 * Returns: SUCCESS or ERROR
 */
int process_instruction(const char *operation, const char *operands, const char *label,
                        int line_num, SymbolTable *symbol_table, MemoryImage *memory);

/*
 * Process .data directive
 * Encodes integers to data image
 */
int process_data_directive(const char *params, int line_num, MemoryImage *memory);

/*
 * Process .string directive
 * Encodes string to data image (ASCII + null terminator)
 */
int process_string_directive(const char *params, int line_num, MemoryImage *memory);

/*
 * Process .entry directive
 * Marks symbol for entry (processed in second pass)
 */
int process_entry_directive(const char *params, int line_num);

/*
 * Process .extern directive
 * Adds external symbol to symbol table
 */
int process_extern_directive(const char *params, int line_num, SymbolTable *symbol_table);

/*
 * Get operation information (opcode, funct, operand count)
 * 
 * Parameters:
 *   operation - Operation name
 *   opcode - Output: operation code
 *   funct - Output: function code
 *   num_operands - Output: number of operands (0, 1, or 2)
 * 
 * Returns: TRUE if valid operation, FALSE otherwise
 */
int get_operation_info(const char *operation, int *opcode, int *funct, int *num_operands);

/*
 * Calculate instruction length in words
 * 
 * Parameters:
 *   operation - Operation name
 *   source - Source operand (can be NULL)
 *   dest - Destination operand (can be NULL)
 * 
 * Returns: Number of memory words needed
 */
int calculate_instruction_length(const char *operation, const char *source, const char *dest);

#endif /* FIRST_PASS_H */