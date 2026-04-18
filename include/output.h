#ifndef OUTPUT_H
#define OUTPUT_H

#include "structures.h"
#include "globals.h"

bool create_object_file(const char *base_name, MemoryImage *memory);

bool create_entries_file(const char *base_name, SymbolTable *symbols);

bool create_externals_file(const char *base_name, ExternalList *externals);

#endif
