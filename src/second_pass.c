#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "second_pass.h"
#include "structures.h"
#include "parser.h"
#include "file_handler.h"
#include "error.h"
#include "string_utils.h"
#include "globals.h"

static SymbolTable *sym_table;
static MemoryImage *memory;
static ExternalList ext_list;
static int current_IC;

typedef struct {
    char name[10];
    int num_ops;
} operation_info;

static operation_info op_table[] = {
    {"mov", 2},
    {"cmp", 2},
    {"add", 2},
    {"sub", 2},
    {"lea", 2},
    {"clr", 1},
    {"not", 1},
    {"inc", 1},
    {"dec", 1},
    {"jmp", 1},
    {"bne", 1},
    {"jsr", 1},
    {"red", 1},
    {"prn", 1},
    {"rts", 0},
    {"stop", 0},
    {"", -1}
};

static operation_info *find_operation(const char *name) {
    int i;
    for (i = 0; op_table[i].num_ops != -1; i++) {
        if (strcmp(op_table[i].name, name) == 0) {
            return &op_table[i];
        }
    }
    return NULL;
}

static void add_external_usage(const char *name, int addr) {
    ExternalRef *new_ref;
    ExternalRef *current;
    
    new_ref = (ExternalRef *)malloc(sizeof(ExternalRef));
    if (new_ref == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    strcpy(new_ref->symbol, name);
    new_ref->address = addr;
    new_ref->next = NULL;
    
    if (ext_list.head == NULL) {
        ext_list.head = new_ref;
    } else {
        current = ext_list.head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_ref;
    }
    ext_list.count++;
}

static int encode_direct_address(const char *label, int line_num) {
    Symbol *sym;
    int word;
    
    sym = symbol_table_find(sym_table, label);
    
    if (sym == NULL) {
        report_error(line_num, "Undefined symbol");
        return 0;
    }
    
    if (sym->attributes & ATTR_EXTERNAL) {
        add_external_usage(label, current_IC);
        word = 0;
    } else {
        word = sym->value;
    }
    
    return word;
}

static int encode_relative_address(const char *label, int line_num) {
    Symbol *sym;
    int distance;
    char clean_label[MAX_LABEL_LENGTH + 1];
    
    strcpy(clean_label, label + 1);
    
    sym = symbol_table_find(sym_table, clean_label);
    
    if (sym == NULL) {
        report_error(line_num, "Undefined symbol");
        return 0;
    }
    
    if (sym->attributes & ATTR_EXTERNAL) {
        report_error(line_num, "Cannot use external symbol with relative addressing");
        return 0;
    }
    
    distance = sym->value - current_IC;
    
    return distance & 0xFFF;
}

static bool process_entry_directive(const char *params, int line_num) {
    Symbol *sym;
    char label[MAX_LABEL_LENGTH + 1];
    const char *p = params;
    
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    
    strcpy(label, p);
    
    sym = symbol_table_find(sym_table, label);
    
    if (sym == NULL) {
        report_error(line_num, "Entry symbol not defined");
        return FALSE;
    }
    
    if (sym->attributes & ATTR_EXTERNAL) {
        report_error(line_num, "Cannot declare external symbol as entry");
        return FALSE;
    }
    
    symbol_table_update_attributes(sym_table, label, ATTR_ENTRY);
    
    return TRUE;
}

static int get_addressing_mode(const char *operand) {
    if (operand == NULL || operand[0] == '\0') {
        return -1;
    }
    
    if (operand[0] == '#') {
        return MODE_IMMEDIATE;
    }
    
    if (operand[0] == 'r' && operand[1] >= '0' && 
        operand[1] <= '7' && operand[2] == '\0') {
        return MODE_REGISTER;
    }
    
    if (operand[0] == '%') {
        return MODE_RELATIVE;
    }
    
    return MODE_DIRECT;
}

static bool process_instruction(const char *operation, const char *op1, const char *op2, int line_num) {
    operation_info *op;
    int src_mode = 0;
    int dst_mode = 0;
    int word;
    
    op = find_operation(operation);
    if (op == NULL) {
        return TRUE;
    }
    
    current_IC = current_IC + 1;
    
    if (op->num_ops == 2) {
        src_mode = get_addressing_mode(op1);
        dst_mode = get_addressing_mode(op2);
        
        if (src_mode == MODE_DIRECT) {
            word = encode_direct_address(op1, line_num);
            memory->code[current_IC - 100].word = word;
        } else if (src_mode == MODE_RELATIVE) {
            word = encode_relative_address(op1, line_num);
            memory->code[current_IC - 100].word = word;
        }
        current_IC = current_IC + 1;
        
        if (dst_mode == MODE_DIRECT) {
            word = encode_direct_address(op2, line_num);
            memory->code[current_IC - 100].word = word;
        } else if (dst_mode == MODE_RELATIVE) {
            word = encode_relative_address(op2, line_num);
            memory->code[current_IC - 100].word = word;
        }
        current_IC = current_IC + 1;
    } else if (op->num_ops == 1) {
        dst_mode = get_addressing_mode(op1);
        
        if (dst_mode == MODE_DIRECT) {
            word = encode_direct_address(op1, line_num);
            memory->code[current_IC - 100].word = word;
        } else if (dst_mode == MODE_RELATIVE) {
            word = encode_relative_address(op1, line_num);
            memory->code[current_IC - 100].word = word;
        }
        current_IC = current_IC + 1;
    }
    
    return TRUE;
}

bool execute_second_pass(const char *filename, SymbolTable *symbols,
                        MemoryImage *mem) {
    FILE *file;
    char line_buffer[MAX_LINE_LENGTH];
    int line_num;
    bool success;
    char *trimmed;
    char *label_end;
    char *operation;
    char *op1;
    char *op2;
    
    printf("\n=== Second Pass ===\n");
    printf("Processing file: %s\n", filename);
    
    sym_table = symbols;
    memory = mem;
    current_IC = 100;
    ext_list.head = NULL;
    ext_list.count = 0;
    
    file = fopen(filename, "r");
    if (file == NULL) {
        report_error(0, "Cannot open file");
        return FALSE;
    }
    
    line_num = 0;
    success = TRUE;
    
    while (fgets(line_buffer, MAX_LINE_LENGTH, file) != NULL) {
        line_num = line_num + 1;
        
        line_buffer[strcspn(line_buffer, "\n")] = '\0';
        
        trimmed = trim_whitespace(line_buffer);
        
        if (trimmed[0] == '\0' || trimmed[0] == ';') {
            continue;
        }
        
        label_end = strchr(trimmed, ':');
        if (label_end != NULL) {
            trimmed = label_end + 1;
            trimmed = trim_whitespace(trimmed);
        }
        
        if (trimmed[0] == '.') {
            if (strncmp(trimmed, ".entry", 6) == 0) {
                if (!process_entry_directive(trimmed + 6, line_num)) {
                    success = FALSE;
                }
            }
            continue;
        }
        
        operation = strtok(trimmed, " \t,");
        if (operation == NULL) {
            continue;
        }
        
        op1 = strtok(NULL, " \t,");
        op2 = strtok(NULL, " \t,");
        
        if (!process_instruction(operation, op1, op2, line_num)) {
            success = FALSE;
        }
    }
    
    fclose(file);
    
    if (success) {
        printf("\nSecond pass completed successfully!\n");
    } else {
        printf("\nSecond pass failed with errors.\n");
    }
    
    return success;
}

ExternalList *get_externals_list(void) {
    return &ext_list;
}