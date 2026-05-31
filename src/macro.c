/*
 * macro.c
 * handles macro expansion (pre-assembler stage)
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

/* function prototypes */
static MacroNode* find_macro(MacroNode *head, const char *name);
static MacroNode* add_macro(MacroNode **head, const char *name);
static void free_macros(MacroNode *head);

/*
 * the main pre-assembler function - expands all macros in source file
 * input: path to source .as file, path to output .am file
 * output: success if expansion completed, error if any issues found
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
    char *rest_ptr;     /* pointer for scanning rest of line after first word */
    char next_word[MAX_LABEL_LENGTH + 1]; /* buffer for scanning words after the first */
    int mcro_in_middle; /* flag - true if mcro keyword found after other text */
    
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
    
    /* process source file line by line */
    while (fgets(line, MAX_LINE_LENGTH, in)) {
        line_num++;
        
        /* check line length (error detection)*/
        if (strlen(line) >= MAX_LINE_LENGTH - 1 && line[strlen(line) - 1] != '\n') {
            print_error(line_num, ERR_LINE_TOO_LONG, NULL);
            error_found = TRUE;
            continue;
        }
        
        /* skip empty lines and comments - don't write to output */
        if (is_empty_or_comment(line)) {
            continue;
        }
        
        /* extract first word of current line to identify line type */
        line_ptr = skip_whitespace(line);
        extract_word(line_ptr, word, MAX_LABEL_LENGTH + 1);

        /* check if mcro or mcroend appears after other text on the same line (error detection) */
        if (strcmp(word, MACRO_START) != 0 && strcmp(word, MACRO_END) != 0) {
            mcro_in_middle = FALSE;
            rest_ptr = skip_whitespace(line_ptr + strlen(word));
            while (*rest_ptr != '\0' && *rest_ptr != '\n') {
                extract_word(rest_ptr, next_word, MAX_LABEL_LENGTH + 1);
                if (strlen(next_word) == 0) break;
                if (strcmp(next_word, MACRO_START) == 0 || strcmp(next_word, MACRO_END) == 0) {
                    print_error(line_num, ERR_NONE, "text before mcro keyword is not allowed");
                    error_found = TRUE;
                    mcro_in_middle = TRUE;
                    break;
                }
                rest_ptr = skip_whitespace(rest_ptr + strlen(next_word));
            }
            if (mcro_in_middle) continue;
        }
        
        /* check for macro definition start */
        if (strcmp(word, MACRO_START) == 0) {
            char macro_name[MAX_LABEL_LENGTH + 1];
            
            /* extract macro name */
            line_ptr = skip_whitespace(line_ptr + strlen(word));    /* skip the "mcro" keyword to get the macro name */
            extract_word(line_ptr, macro_name, MAX_LABEL_LENGTH + 1);
            
            /* validate macro name (error detection) */
            if (!is_valid_label(macro_name)) {
                print_error(line_num, ERR_MACRO_NAME, macro_name);
                error_found = TRUE;
                continue;
            }
            
            /* check for duplicate macro (error detection) */
            if (find_macro(macro_table, macro_name) != NULL) {
                print_error(line_num, ERR_DUPLICATE_LABEL, macro_name);
                error_found = TRUE;
                continue;
            }
            
            /* create new macro */
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
            /* check for extraneous text after mcroend (error detection) */
            rest_ptr = skip_whitespace(line_ptr + strlen(word));
            if (*rest_ptr != '\0' && *rest_ptr != '\n') {
                print_error(line_num, ERR_NONE, "text after mcroend keyword is not allowed");
                error_found = TRUE;
                in_macro_definition = FALSE;
                current_macro = NULL;
                continue;
            }
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
        
        /* if currently defining a macro, add current line to macro */
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
        
        /* check if this is a macro call */
        found_macro = find_macro(macro_table, word);
        if (found_macro != NULL) {
            /* macro call - expand macro write all macro lines to output file */
            for (i = 0; i < found_macro->line_count; i++) {
                fputs(found_macro->lines[i], out);
            }
        } else {
            /* regular line - write to output file as is*/
            fputs(line, out);
        }
    }
    
    /* check for unclosed macro (error detection) */
    if (in_macro_definition) {
        print_error(line_num, ERR_MACRO_UNCLOSED, current_macro->name);
        error_found = TRUE;
    }
    
    /* cleanup */
    fclose(in);
    fclose(out);
    free_macros(macro_table);
    
    if (error_found) {
        return ERROR;
    }
    
    return SUCCESS;
}

/*
 * searches the macro table for a macro by name
 * input: head of macro list, name to search for
 * output: pointer to matching MacroNode, NULL if not found
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
 * adds a new macro to the macro table
 * input: pointer to head of macro list, name of new macro
 * output: pointer to the new MacroNode, NULL if memory allocation failed
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
 * frees all dynamically allocated macro nodes in the macro table
 * input: head of macro list
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