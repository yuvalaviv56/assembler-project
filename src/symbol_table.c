/*
 * symbol_table.c
 * Implementation of symbol table operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "string_utils.h"
#include "constants.h"
#include "globals.h"

/* Initialize an empty symbol table */
void symbol_table_init(SymbolTable *table) {
    if (table == NULL) {
        return;
    }
    
    table->head = NULL;
    table->count = 0;
}

/* Add a new symbol to the table */
int symbol_table_add(SymbolTable *table, const char *name, int value, unsigned int attributes) {
    Symbol *new_symbol;
    
    if (table == NULL || name == NULL) {
        return FALSE;
    }
    
    /* Check if symbol already exists */
    if (symbol_table_find(table, name) != NULL) {
        return FALSE; /* Duplicate symbol */
    }
    
    /* Allocate new symbol */
    new_symbol = (Symbol*)malloc(sizeof(Symbol));
    if (new_symbol == NULL) {
        return FALSE; /* Memory allocation failed */
    }
    
    /* Initialize symbol */
    safe_strcpy(new_symbol->name, name, MAX_LABEL_LENGTH + 1);
    new_symbol->value = value;
    new_symbol->attributes = attributes;
    
    /* Insert at head of list */
    new_symbol->next = table->head;
    table->head = new_symbol;
    table->count++;
    
    return TRUE;
}

/* Find a symbol in the table */
Symbol* symbol_table_find(SymbolTable *table, const char *name) {
    Symbol *current;
    
    if (table == NULL || name == NULL) {
        return NULL;
    }
    
    current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL; /* Not found */
}

/* Update a symbol's attributes */
int symbol_table_update_attributes(SymbolTable *table, const char *name, unsigned int attributes) {
    Symbol *symbol = symbol_table_find(table, name);
    
    if (symbol == NULL) {
        return FALSE; /* Symbol not found */
    }
    
    /* Add new attributes (bitwise OR) */
    symbol->attributes |= attributes;
    
    return TRUE;
}

/* Update a symbol's value */
int symbol_table_update_value(SymbolTable *table, const char *name, int value) {
    Symbol *symbol = symbol_table_find(table, name);
    
    if (symbol == NULL) {
        return FALSE; /* Symbol not found */
    }
    
    symbol->value = value;
    
    return TRUE;
}

/* Update all data symbols by adding offset */
void symbol_table_update_data_symbols(SymbolTable *table, int offset) {
    Symbol *current;
    
    if (table == NULL) {
        return;
    }
    
    current = table->head;
    while (current != NULL) {
        /* If symbol has ATTR_DATA attribute, add offset to its value */
        if (current->attributes & ATTR_DATA) {
            current->value += offset;
        }
        current = current->next;
    }
}

/* Check if a symbol has a specific attribute */
int symbol_has_attribute(const Symbol *symbol, SymbolAttribute attribute) {
    if (symbol == NULL) {
        return FALSE;
    }
    
    return (symbol->attributes & attribute) != 0;
}

/* Print symbol table (for debugging) */
void symbol_table_print(const SymbolTable *table) {
    Symbol *current;
    
    if (table == NULL) {
        printf("Symbol table is NULL\n");
        return;
    }
    
    printf("\n========== SYMBOL TABLE ==========\n");
    printf("Total symbols: %d\n\n", table->count);
    printf("%-15s %-10s %-20s\n", "Name", "Value", "Attributes");
    printf("%-15s %-10s %-20s\n", "----", "-----", "----------");
    
    current = table->head;
    while (current != NULL) {
        char attrs[50] = "";
        
        /* Build attribute string */
        if (current->attributes & ATTR_CODE) {
            strcat(attrs, "CODE ");
        }
        if (current->attributes & ATTR_DATA) {
            strcat(attrs, "DATA ");
        }
        if (current->attributes & ATTR_ENTRY) {
            strcat(attrs, "ENTRY ");
        }
        if (current->attributes & ATTR_EXTERNAL) {
            strcat(attrs, "EXTERNAL ");
        }
        if (current->attributes == ATTR_NONE) {
            strcat(attrs, "NONE");
        }
        
        printf("%-15s %-10d %-20s\n", current->name, current->value, attrs);
        
        current = current->next;
    }
    
    printf("==================================\n\n");
}

/* Free all symbols in the table */
void symbol_table_free(SymbolTable *table) {
    Symbol *current;
    Symbol *next;
    
    if (table == NULL) {
        return;
    }
    
    current = table->head;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    
    table->head = NULL;
    table->count = 0;
}