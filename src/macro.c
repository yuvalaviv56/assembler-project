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

/* macro table entry (linked list node) */
typedef struct MacroNode {
    char name[MAX_LABEL_LENGTH + 1];     /* macro name (+1 for null terminator) */
    char lines[MAX_MACRO_LINES][MAX_LINE_LENGTH]; /* stored lines of the macro body */
    int line_count;  /* number of lines in macro body */
    struct MacroNode *next;  /* pointer to next macro in the table */
} MacroNode;

/* Function prototypes */
static MacroNode* find_macro(MacroNode *head, const char *name);
static MacroNode* add_macro(MacroNode **head, const char *name);
static void free_macros(MacroNode *head);

/*
 * The main pre-assembler function - expands all macros in source file
 * Input: path to source .as file, path to output .am file
 * Output: SUCCESS if expansion completed, ERROR if any issues found
 */
int expand_macros(const char *source, const char *output) {
    FILE *in, *out;     /* input and output file pointers */
    char line[MAX_LINE_LENGTH];      /* buffer array for current line being processed */
    MacroNode *macro_table = NULL;      /* pointer to head of the macro linked list (macro table) */
    MacroNode *current_macro = NULL;    /* pointer to macro currently being defined */
    int in_macro_definition = FALSE;    /* flag (true if currently defining a macro) */
    int line_num = 0;   /* current line number (for error reporting) */
    int error_found = FALSE;    /* flag (true if there was an error) */
    char word[MAX_LABEL_LENGTH + 1]; /* first word of current line (used to identify line type) */
    MacroNode *found_macro;     /* result of macro lookup for current line (pointer to macro or NULL)*/
    int i;      /* counter for macro expansion loop */
    char *line_ptr;     /* pointer for moving through the current line */
    
    /* open source file for reading */
    in = fopen(source, "r");
    if (!in) {
        print_error(0, ERR_FILE_OPEN, source);
        return ERROR;
    }
    
    /* open output file for writing */
    out = fopen(output, "w");
    if (!out) {
        fclose(in);
        print_error(0, ERR_FILE_OPEN, output);
        return ERROR;
    }
    
    /* Process source file line by line */
    while (fgets(line, MAX_LINE_LENGTH, in)) {
        line_num++;
        
        /* Check line length (error detection)*/
        if (strlen(line) >= MAX_LINE_LENGTH - 1 && line[strlen(line) - 1] != '\n') {
            print_error(line_num, ERR_LINE_TOO_LONG, NULL);
            error_found = TRUE;
            continue;
        }
        
        /* Skip empty lines and comments - don't write to output */
        if (is_empty_or_comment(line)) {
            continue;
        }
        
        /* extract first word of current line to identify line type */
        line_ptr = skip_whitespace(line);
        extract_word(line_ptr, word, MAX_LABEL_LENGTH + 1);
        
        /* Check for macro definition start */
        if (strcmp(word, MACRO_START) == 0) {
            char macro_name[MAX_LABEL_LENGTH + 1];
            
            /* Extract macro name */
            line_ptr = skip_whitespace(line_ptr + strlen(word));    /* skip the "mcro" keyword to get the macro name */
            extract_word(line_ptr, macro_name, MAX_LABEL_LENGTH + 1);
            
            /* Validate macro name (error detection) */
            if (!is_valid_label(macro_name)) {
                print_error(line_num, ERR_MACRO_NAME, macro_name);
                error_found = TRUE;
                continue;
            }
            
            /* Check for duplicate macro (error detection) */
            if (find_macro(macro_table, macro_name) != NULL) {
                print_error(line_num, ERR_DUPLICATE_LABEL, macro_name);
                error_found = TRUE;
                continue;
            }
            
            /* Create new macro */
            current_macro = add_macro(&macro_table, macro_name);
            /* if memory allocation failed clean up and exit (error detection) */
            if (current_macro == NULL) {
                print_error(line_num, ERR_NONE, "Failed to allocate memory for macro");
                error_found = TRUE;
                fclose(in);
                fclose(out);
                free_macros(macro_table);
                return ERROR;
            }
            
            /* update the flag to reflect that a macro definition is in progress */
            in_macro_definition = TRUE;
            continue;
        }
        
        /* Check for macro definition end */
        if (strcmp(word, MACRO_END) == 0) {
             /* mcroend found while not defining a macro (error detection) */
            if (!in_macro_definition) {
                print_error(line_num, ERR_NONE, "mcroend without mcro");
                error_found = TRUE;
            }
            /* in case macro ended reset macro definition state and continue */
            in_macro_definition = FALSE;
            current_macro = NULL;
            continue;
        }
        
        /* If currently defining a macro, add current line to macro */
        if (in_macro_definition) {
            /* check if macro body exceeds maximum length (error detection) */
            if (current_macro->line_count >= MAX_MACRO_LINES) {
                print_error(line_num, ERR_NONE, "Macro too long");
                error_found = TRUE;
                continue;
            }
            /* line passed validity checks - add to macro body and increase line counter */
            safe_strcpy(current_macro->lines[current_macro->line_count], line, MAX_LINE_LENGTH);
            current_macro->line_count++;
            continue;
        }
        
        /* Check if this is a macro call */
        found_macro = find_macro(macro_table, word);
        if (found_macro != NULL) {
            /* Macro call - expand macro write all macro lines to output file */
            for (i = 0; i < found_macro->line_count; i++) {
                fputs(found_macro->lines[i], out);
            }
        } else {
            /* Regular line - write to output file as is*/
            fputs(line, out);
        }
    }
    
    /* Check for unclosed macro (error detection) */
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

/*
 * Searches the macro table for a macro by name
 * Input: head of macro list, name to search for
 * Output: pointer to matching MacroNode, NULL if not found
 */
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

/*
 * Adds a new macro to the macro table
 * Input: pointer to head of macro list, name of new macro
 * Output: pointer to the new MacroNode, NULL if memory allocation failed
 */
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

/*
 * Frees all dynamically allocated macro nodes in the macro table
 * Input: head of macro list
 */
static void free_macros(MacroNode *head) {
    MacroNode *current = head;
    MacroNode *next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}