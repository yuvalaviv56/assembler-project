/*
 * test_symbol_table.c
 * Test program for symbol table implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include "symbol_table.h"
#include "constants.h"

void test_basic_operations(void);
void test_duplicate_handling(void);
void test_attribute_updates(void);
void test_data_symbol_update(void);

int main(void) {
    printf("========================================\n");
    printf("Symbol Table Unit Tests\n");
    printf("========================================\n\n");
    
    test_basic_operations();
    test_duplicate_handling();
    test_attribute_updates();
    test_data_symbol_update();
    
    printf("========================================\n");
    printf("All tests completed!\n");
    printf("========================================\n");
    
    return 0;
}

void test_basic_operations(void) {
    SymbolTable table;
    Symbol *found;
    
    printf("Test 1: Basic Operations\n");
    printf("-------------------------\n");
    
    symbol_table_init(&table);
    printf("✓ Initialized empty table\n");
    
    if (symbol_table_add(&table, "MAIN", 100, ATTR_CODE)) {
        printf("✓ Added MAIN at 100 (code)\n");
    }
    
    if (symbol_table_add(&table, "LOOP", 103, ATTR_CODE)) {
        printf("✓ Added LOOP at 103 (code)\n");
    }
    
    if (symbol_table_add(&table, "STR", 0, ATTR_DATA)) {
        printf("✓ Added STR at 0 (data)\n");
    }
    
    found = symbol_table_find(&table, "MAIN");
    if (found != NULL && found->value == 100) {
        printf("✓ Found MAIN with value 100\n");
    }
    
    found = symbol_table_find(&table, "NONEXISTENT");
    if (found == NULL) {
        printf("✓ Correctly returned NULL for nonexistent symbol\n");
    }
    
    symbol_table_print(&table);
    
    symbol_table_free(&table);
    printf("✓ Freed symbol table\n\n");
}

void test_duplicate_handling(void) {
    SymbolTable table;
    
    printf("Test 2: Duplicate Handling\n");
    printf("---------------------------\n");
    
    symbol_table_init(&table);
    
    if (symbol_table_add(&table, "TEST", 100, ATTR_CODE)) {
        printf("✓ Added TEST\n");
    }
    
    if (!symbol_table_add(&table, "TEST", 200, ATTR_CODE)) {
        printf("✓ Correctly rejected duplicate symbol TEST\n");
    }
    
    symbol_table_free(&table);
    printf("\n");
}

void test_attribute_updates(void) {
    SymbolTable table;
    Symbol *symbol;
    
    printf("Test 3: Attribute Updates\n");
    printf("--------------------------\n");
    
    symbol_table_init(&table);
    
    symbol_table_add(&table, "FUNC", 100, ATTR_CODE);
    
    if (symbol_table_update_attributes(&table, "FUNC", ATTR_ENTRY)) {
        printf("✓ Added ENTRY attribute to FUNC\n");
    }
    
    symbol = symbol_table_find(&table, "FUNC");
    if (symbol != NULL && 
        symbol_has_attribute(symbol, ATTR_CODE) && 
        symbol_has_attribute(symbol, ATTR_ENTRY)) {
        printf("✓ FUNC has both CODE and ENTRY attributes\n");
    }
    
    symbol_table_print(&table);
    
    symbol_table_free(&table);
    printf("\n");
}

void test_data_symbol_update(void) {
    SymbolTable table;
    Symbol *symbol;
    
    printf("Test 4: Data Symbol Update\n");
    printf("---------------------------\n");
    
    symbol_table_init(&table);
    
    symbol_table_add(&table, "MAIN", 100, ATTR_CODE);
    symbol_table_add(&table, "LOOP", 103, ATTR_CODE);
    symbol_table_add(&table, "STR", 0, ATTR_DATA);
    symbol_table_add(&table, "NUM", 5, ATTR_DATA);
    
    printf("Before update:\n");
    symbol_table_print(&table);
    
    symbol_table_update_data_symbols(&table, 110);
    printf("After updating data symbols with offset 110:\n");
    
    symbol = symbol_table_find(&table, "MAIN");
    if (symbol != NULL && symbol->value == 100) {
        printf("✓ MAIN unchanged at 100 (code symbol)\n");
    }
    
    symbol = symbol_table_find(&table, "STR");
    if (symbol != NULL && symbol->value == 110) {
        printf("✓ STR updated to 110 (was 0, +110)\n");
    }
    
    symbol = symbol_table_find(&table, "NUM");
    if (symbol != NULL && symbol->value == 115) {
        printf("✓ NUM updated to 115 (was 5, +110)\n");
    }
    
    symbol_table_print(&table);
    
    symbol_table_free(&table);
    printf("\n");
}