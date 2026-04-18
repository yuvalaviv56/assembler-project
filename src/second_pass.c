#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "second_pass.h"
#include "symbol_table.h"
#include "parser.h"
#include "file_handler.h"
#include "operand_encoder.h"
#include "string_utils.h"
#include "errors.h"
#include "first_pass.h"
#include "globals.h"

static SymbolTable *sym_table;
static MemoryImage *memory;
static ExternalList *ext_list;
static int current_IC;
static ExternalRef *externals_head = NULL;

static void add_external_ref(const char *symbol, int address) {
    ExternalRef *new_ref;
    new_ref = (ExternalRef*)malloc(sizeof(ExternalRef));
    if (new_ref == NULL) return;
    strcpy(new_ref->symbol, symbol);
    new_ref->address = address;
    new_ref->next = externals_head;
    externals_head = new_ref;
    ext_list->count = ext_list->count + 1;
}

static void encode_direct_addr(const char *label, int line_num) {
    Symbol *sym;
    if (label == NULL || *label == '\0') return;
    sym = symbol_table_find(sym_table, label);
    if (sym == NULL) {
        print_error(line_num, ERR_UNDEFINED_LABEL, label);
        return;
    }
    if (sym->attributes & ATTR_EXTERNAL) {
        memory->code[current_IC].word = 0;
        memory->code[current_IC].are = ARE_EXTERNAL;
        add_external_ref(label, 100 + current_IC);
    } else {
        memory->code[current_IC].word = sym->value & 0xFFF;
        memory->code[current_IC].are = ARE_RELOCATABLE;
    }
    current_IC = current_IC + 1;
}

static void encode_relative_addr(const char *label, int line_num) {
    Symbol *sym;
    char clean_label[MAX_LABEL_LENGTH + 1];
    int offset;
    if (label == NULL || *label == '\0') return;
    strcpy(clean_label, label + 1);
    sym = symbol_table_find(sym_table, clean_label);
    if (sym == NULL) {
        print_error(line_num, ERR_UNDEFINED_LABEL, clean_label);
        return;
    }
    offset = sym->value - (100 + current_IC);
    memory->code[current_IC].word = offset & 0xFFF;
    memory->code[current_IC].are = ARE_ABSOLUTE;
    current_IC = current_IC + 1;
}

static void handle_entry_directive(const char *params, int line_num) {
    char label[MAX_LABEL_LENGTH + 1];
    Symbol *sym;
    if (params == NULL || *params == '\0') {
        print_error(line_num, ERR_EMPTY_DIRECTIVE, NULL);
        return;
    }
    strcpy(label, params);
    trim(label);
    sym = symbol_table_find(sym_table, label);
    if (sym == NULL) {
        print_error(line_num, ERR_UNDEFINED_LABEL, label);
        return;
    }
    if (sym->attributes & ATTR_EXTERNAL) {
        print_error(line_num, ERR_NONE, "Cannot mark external symbol as entry");
        return;
    }
    symbol_table_update_attributes(sym_table, label, ATTR_ENTRY);
}

static void handle_instruction(const char *operation, const char *operands, int line_num) {
    char source[MAX_LINE_LENGTH];
    char dest[MAX_LINE_LENGTH];
    char operands_copy[MAX_LINE_LENGTH];
    int num_operands, opcode, funct, num_expected;
    AddressingMode src_mode, dest_mode;
    
    if (!get_operation_info(operation, &opcode, &funct, &num_expected)) return;
    current_IC = current_IC + 1;
    strcpy(operands_copy, operands);
    num_operands = parse_operands(operands_copy, source, dest, MAX_LINE_LENGTH);
    if (num_operands != num_expected) return;
    if (num_expected == 0) return;
    
    if (num_expected == 2) {
        src_mode = identify_addressing_mode(source);
        dest_mode = identify_addressing_mode(dest);
        if (src_mode == MODE_REGISTER && dest_mode == MODE_REGISTER) {
            current_IC = current_IC + 1;
        } else {
            if (src_mode == MODE_DIRECT) encode_direct_addr(source, line_num);
            else if (src_mode == MODE_RELATIVE) encode_relative_addr(source, line_num);
            else current_IC = current_IC + 1;
            
            if (dest_mode == MODE_DIRECT) encode_direct_addr(dest, line_num);
            else if (dest_mode == MODE_RELATIVE) encode_relative_addr(dest, line_num);
            else current_IC = current_IC + 1;
        }
    } else if (num_expected == 1) {
        dest_mode = identify_addressing_mode(dest);
        if (dest_mode == MODE_DIRECT) encode_direct_addr(dest, line_num);
        else if (dest_mode == MODE_RELATIVE) encode_relative_addr(dest, line_num);
        else current_IC = current_IC + 1;
    }
}

bool execute_second_pass(const char *filename, SymbolTable *symbols, MemoryImage *mem, ExternalList *externals) {
    FILE *file;
    char line_buffer[MAX_LINE_LENGTH];
    char label[MAX_LABEL_LENGTH + 1];
    char directive[MAX_LABEL_LENGTH + 1];
    char operation[MAX_LABEL_LENGTH + 1];
    char params[MAX_LINE_LENGTH];
    char operands[MAX_LINE_LENGTH];
    char *line_after_label, *trimmed;
    int line_num = 0, error_found = FALSE;
    LineType line_type;
    
    sym_table = symbols;
    memory = mem;
    ext_list = externals;
    current_IC = 0;
    externals_head = NULL;
    
    file = fopen(filename, "r");
    if (file == NULL) {
        print_error(0, ERR_FILE_OPEN, filename);
        return FALSE;
    }
    
    while (fgets(line_buffer, MAX_LINE_LENGTH, file)) {
        line_num = line_num + 1;
        trimmed = line_buffer;
        trim(trimmed);
        if (is_empty_or_comment(trimmed)) continue;
        line_type = parse_line_type(trimmed);
        if (line_type == LINE_EMPTY) continue;
        line_after_label = parse_label(trimmed, label, MAX_LABEL_LENGTH + 1);
        trimmed = line_after_label;
        trim(trimmed);
        
        if (line_type == LINE_DIRECTIVE) {
            if (parse_directive(trimmed, directive, params, MAX_LINE_LENGTH)) {
                if (strcmp(directive, DIR_ENTRY) == 0) {
                    handle_entry_directive(params, line_num);
                }
            }
        } else if (line_type == LINE_INSTRUCTION) {
            if (parse_instruction(trimmed, operation, operands, MAX_LINE_LENGTH)) {
                handle_instruction(operation, operands, line_num);
            }
        }
    }
    
    fclose(file);
    ext_list->head = externals_head;
    return error_found ? FALSE : TRUE;
}
