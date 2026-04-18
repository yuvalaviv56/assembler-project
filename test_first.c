#include <stdio.h>
#include "first_pass.h"
#include "symbol_table.h"

int main() {
    SymbolTable table;
    MemoryImage memory;
    symbol_table_init(&table);
    int result = first_pass("tests/test1.am", &table, &memory);
    printf("first_pass returned: %d\n", result);
    symbol_table_free(&table);
    return 0;
}
