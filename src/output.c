#include <stdio.h>
#include <string.h>
#include "output.h"
#include "structures.h"
#include "globals.h"

static void to_hex(int value, char *output) {
    sprintf(output, "%03X", value & 0xFFF);
}

bool create_object_file(const char *base_name, MemoryImage *memory) {
    FILE *fp;
    char filename[256];
    char hex[4];
    int i, address;
    
    sprintf(filename, "%s.ob", base_name);
    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Cannot create file: %s\n", filename);
        return FALSE;
    }
    
    fprintf(fp, "%d %d\n", memory->ICF - 100, memory->DCF);
    
    for (i = 0; i < memory->ICF - 100; i++) {
        address = 100 + i;
        to_hex(memory->code[i].word, hex);
        fprintf(fp, "%04d %s\n", address, hex);
    }
    
    for (i = 0; i < memory->DCF; i++) {
        address = memory->ICF + i;
        to_hex(memory->data[i].word, hex);
        fprintf(fp, "%04d %s\n", address, hex);
    }
    
    fclose(fp);
    printf("Created: %s\n", filename);
    return TRUE;
}

bool create_entries_file(const char *base_name, SymbolTable *symbols) {
    FILE *fp;
    char filename[256];
    Symbol *current;
    int has_entries = FALSE;
    
    current = symbols->head;
    while (current != NULL) {
        if (current->attributes & ATTR_ENTRY) {
            has_entries = TRUE;
            break;
        }
        current = current->next;
    }
    
    if (has_entries == FALSE) {
        return TRUE;
    }
    
    sprintf(filename, "%s.ent", base_name);
    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Cannot create file: %s\n", filename);
        return FALSE;
    }
    
    current = symbols->head;
    while (current != NULL) {
        if (current->attributes & ATTR_ENTRY) {
            fprintf(fp, "%s %04d\n", current->name, current->value);
        }
        current = current->next;
    }
    
    fclose(fp);
    printf("Created: %s\n", filename);
    return TRUE;
}

bool create_externals_file(const char *base_name, ExternalList *externals) {
    FILE *fp;
    char filename[256];
    ExternalRef *current;
    
    current = (ExternalRef*)externals->head;
    
    if (current == NULL) {
        return TRUE;
    }
    
    sprintf(filename, "%s.ext", base_name);
    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Cannot create file: %s\n", filename);
        return FALSE;
    }
    
    while (current != NULL) {
        fprintf(fp, "%s %04d\n", current->symbol, current->address);
        current = current->next;
    }
    
    fclose(fp);
    printf("Created: %s\n", filename);
    return TRUE;
}
