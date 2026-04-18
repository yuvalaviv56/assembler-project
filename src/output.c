#include <stdio.h>
#include <string.h>
#include "output.h"
#include "structures.h"
#include "globals.h"

/* המרה להקסדצימלי */
static void to_hex(int value, char *output) {
    sprintf(output, "%03X", value & 0xFFF);
}

/* יצירת קובץ .ob */
bool create_object_file(const char *base_name, int *code_img, int *data_img,
                       int ic, int dc) {
    FILE *fp;
    char filename[256];
    char hex[4];
    int i;
    int address;
    
    sprintf(filename, "%s.ob", base_name);
    
    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Cannot create file: %s\n", filename);
        return FALSE;
    }
    
    fprintf(fp, "%d %d\n", ic - 100, dc);
    
    for (i = 0; i < ic - 100; i++) {
        address = 100 + i;
        to_hex(code_img[i], hex);
        fprintf(fp, "%04d %s A\n", address, hex);
    }
    
    for (i = 0; i < dc; i++) {
        address = ic + i;
        to_hex(data_img[i], hex);
        fprintf(fp, "%04d %s A\n", address, hex);
    }
    
    fclose(fp);
    
    printf("Created: %s\n", filename);
    return TRUE;
}

/* יצירת קובץ .ent */
bool create_entries_file(const char *base_name, SymbolTable *symbols) {
    FILE *fp;
    char filename[256];
    Symbol *current;
    int has_entries;
    
    has_entries = FALSE;
    current = symbols->head;;
    
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

/* extern declaration */
typedef struct ext_node {
    char symbol_name[MAX_LABEL_LENGTH + 1];
    int address;
    struct ext_node *next;
} ext_node;

extern ext_node *get_externals_list(void);

/* יצירת קובץ .ext */
bool create_externals_file(const char *base_name) {
    FILE *fp;
    char filename[256];
    ext_node *current;
    
    current = get_externals_list();
    
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
        fprintf(fp, "%s %04d\n", current->symbol_name, current->address);
        current = current->next;
    }
    
    fclose(fp);
    
    printf("Created: %s\n", filename);
    return TRUE;
}