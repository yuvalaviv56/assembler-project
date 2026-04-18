#ifndef OUTPUT_H
#define OUTPUT_H

#include "globals.h"
#include "symbol_table.h"

/*
 * יצירת קובץ .ob
 */
bool create_object_file(const char *base_name, int *code_img, int *data_img, 
                        int ic, int dc);

/*
 * יצירת קובץ .ent
 */
bool create_entries_file(const char *base_name, SymbolTable *symbols);

/*
 * יצירת קובץ .ext
 */
bool create_externals_file(const char *base_name);

#endif