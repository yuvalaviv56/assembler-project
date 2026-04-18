#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "structures.h"
#include "globals.h"

bool execute_second_pass(const char *filename, SymbolTable *symbols, MemoryImage *memory, ExternalList *externals);

#endif