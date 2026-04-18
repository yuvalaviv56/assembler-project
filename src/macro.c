/*
 * macro.c
 * Handles macro expansion (pre-assembler stage)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "structures.h"
#include "macro.h"
#include "string_utils.h"
#include "errors.h"
#include "globals.h"

/* Macro table structure (simple linked list) */
typedef struct MacroNode {
    char name[MAX_LABEL_LENGTH + 1];
    char lines[MAX_MACRO_LINES][MAX_LINE_LENGTH];
    int line_count;
    struct MacroNode *next;
} MacroNode;

/* Function prototypes */
static MacroNode* find_macro(MacroNode *head, const char *name);
static MacroNode* add_macro(MacroNode **head, const char *name);
static void free_macros(MacroNode *head);

/*
 * Main macro expansion function
 * Reads source file, processes macros, writes to output file
 */
int expand_macros(const char *source, const char *output) {
    FILE *in, *out;
    char line[MAX_LINE_LENGTH];
    MacroNode *macro_table = NULL;
    MacroNode *current_macro = NULL;
    int in_macro_definition = FALSE;
    int line_num = 0;
    int error_found = FALSE;
    char word[MAX_LABEL_LENGTH + 1];
    MacroNode *found_macro;
    int i;
    char *line_ptr;
    
    /* Open input file */
    in = fopen(source, "r");
    if (!in) {
        print_error(0, ERR_FILE_OPEN, source);
        return ERROR;
    }
    
    /* Open output file */
    out = fopen(output, "w");
    if (!out) {
        fclose(in);
        print_error(0, ERR_FILE_OPEN, output);
        return ERROR;
    }
    
    /* Process file line by line */
    while (fgets(line, MAX_LINE_LENGTH, in)) {
        line_num++;
        
        /* Check line length */
        if (strlen(line) >= MAX_LINE_LENGTH - 1 && line[strlen(line) - 1] != '\n') {
            print_error(line_num, ERR_LINE_TOO_LONG, NULL);
            error_found = TRUE;
            continue;
        }
        
        /* Skip empty lines and comments - DON'T write them to output */
        if (is_empty_or_comment(line)) {
            continue; /* Skip entirely, don't write to output */
        }
        
        /* Extract first word */
        line_ptr = skip_whitespace(line);
        extract_word(line_ptr, word, MAX_LABEL_LENGTH + 1);
        
        /* Check for macro definition start */
        if (strcmp(word, MACRO_START) == 0) {
            char macro_name[MAX_LABEL_LENGTH + 1];
            
            /* Extract macro name */
            line_ptr = skip_whitespace(line_ptr + strlen(word));
            extract_word(line_ptr, macro_name, MAX_LABEL_LENGTH + 1);
            
            /* Validate macro name */
            if (!is_valid_label(macro_name)) {
                print_error(line_num, ERR_MACRO_NAME, macro_name);
                error_found = TRUE;
                continue;
            }
            
            /* Check for duplicate macro */
            if (find_macro(macro_table, macro_name) != NULL) {
                print_error(line_num, ERR_DUPLICATE_LABEL, macro_name);
                error_found = TRUE;
                continue;
            }
            
            /* Create new macro */
            current_macro = add_macro(&macro_table, macro_name);
            if (current_macro == NULL) {
                print_error(line_num, ERR_NONE, "Failed to allocate memory for macro");
                error_found = TRUE;
                fclose(in);
                fclose(out);
                free_macros(macro_table);
                return ERROR;
            }
            
            in_macro_definition = TRUE;
            continue;
        }
        
        /* Check for macro definition end */
        if (strcmp(word, MACRO_END) == 0) {
            if (!in_macro_definition) {
                print_error(line_num, ERR_NONE, "mcroend without mcro");
                error_found = TRUE;
            }
            in_macro_definition = FALSE;
            current_macro = NULL;
            continue;
        }
        
        /* If inside macro definition, add line to macro */
        if (in_macro_definition) {
            if (current_macro->line_count >= MAX_MACRO_LINES) {
                print_error(line_num, ERR_NONE, "Macro too long");
                error_found = TRUE;
                continue;
            }
            safe_strcpy(current_macro->lines[current_macro->line_count], line, MAX_LINE_LENGTH);
            current_macro->line_count++;
            continue;
        }
        
        /* Check if this is a macro call */
        found_macro = find_macro(macro_table, word);
        if (found_macro != NULL) {
            /* Expand macro - write all macro lines to output */
            for (i = 0; i < found_macro->line_count; i++) {
                fputs(found_macro->lines[i], out);
            }
        } else {
            /* Regular line - write to output */
            fputs(line, out);
        }
    }
    
    /* Check for unclosed macro */
    if (in_macro_definition) {
        print_error(line_num, ERR_MACRO_UNCLOSED, current_macro->name);
        error_found = TRUE;
    }
    
    /* Cleanup */
    fclose(in);
    fclose(out);
    free_macros(macro_table);
    
    return error_found ? ERROR : SUCCESS;
}

/* Find macro by name */
static MacroNode* find_macro(MacroNode *head, const char *name) {
    MacroNode *current = head;
    
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

/* Add new macro to table */
static MacroNode* add_macro(MacroNode **head, const char *name) {
    MacroNode *new_macro = (MacroNode*)malloc(sizeof(MacroNode));
    
    if (new_macro == NULL) {
        return NULL;
    }
    
    safe_strcpy(new_macro->name, name, MAX_LABEL_LENGTH + 1);
    new_macro->line_count = 0;
    new_macro->next = *head;
    *head = new_macro;
    
    return new_macro;
}

/* Free all macros */
static void free_macros(MacroNode *head) {
    MacroNode *current = head;
    MacroNode *next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}