/*
 * symbol_table.h
 * Symbol table operations
 */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "structures.h"

/*
 * Initialize an empty symbol table
 */
void symbol_table_init(SymbolTable *table);

/*
 * Add a new symbol to the table
 * Returns: TRUE if added successfully, FALSE if duplicate or error
 */
int symbol_table_add(SymbolTable *table, const char *name, int value, unsigned int attributes);

/*
 * Find a symbol in the table
 * Returns: Pointer to symbol if found, NULL otherwise
 */
Symbol* symbol_table_find(SymbolTable *table, const char *name);

/*
 * Update a symbol's attributes (e.g., add ATTR_ENTRY)
 * Returns: TRUE if updated, FALSE if not found
 */
int symbol_table_update_attributes(SymbolTable *table, const char *name, unsigned int attributes);

/*
 * Update a symbol's value
 * Returns: TRUE if updated, FALSE if not found
 */
int symbol_table_update_value(SymbolTable *table, const char *name, int value);

/*
 * Update all data symbols by adding offset
 * Called after first pass to adjust data addresses
 */
void symbol_table_update_data_symbols(SymbolTable *table, int offset);

/*
 * Check if a symbol has a specific attribute
 * Returns: TRUE if symbol has attribute, FALSE otherwise
 */
int symbol_has_attribute(const Symbol *symbol, SymbolAttribute attribute);

/*
 * Print symbol table (for debugging)
 */
void symbol_table_print(const SymbolTable *table);

/*
 * Free all symbols in the table
 */
void symbol_table_free(SymbolTable *table);

#endif /* SYMBOL_TABLE_H */