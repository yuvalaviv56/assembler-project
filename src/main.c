/*
 * main.c
 * Main entry point for the assembler
 * 
 * Usage: assembler <file1> <file2> ... <fileN>
 * (without .as extension)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "structures.h"
#include "errors.h"
#include "macro.h"
#include "first_pass.h"
#include "second_pass.h"
#include "output.h"
#include "globals.h"

/*
 * Process a single assembly source file
 * Returns SUCCESS or ERROR
 */
int process_file(const char *filename);

/*
 * Main function
 * Processes all assembly files provided as command-line arguments
 */
int main(int argc, char *argv[]) {
    int i;
    int total_errors = 0;
    
    /* Check if any files were provided */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        fprintf(stderr, "Note: Provide filenames without the .as extension\n");
        return ERROR;
    }
    
    /* Process each file */
    printf("Assembler started. Processing %d file(s)...\n\n", argc - 1);
    
    for (i = 1; i < argc; i++) {
        printf("========================================\n");
        printf("Processing: %s.as\n", argv[i]);
        printf("========================================\n");
        
        if (process_file(argv[i]) == ERROR) {
            total_errors++;
            printf("✗ Failed to process %s.as\n\n", argv[i]);
        } else {
            printf("✓ Successfully processed %s.as\n\n", argv[i]);
        }
    }
    
    /* Summary */
    printf("========================================\n");
    printf("Assembly complete.\n");
    printf("Files processed: %d\n", argc - 1);
    printf("Successful: %d\n", argc - 1 - total_errors);
    printf("Failed: %d\n", total_errors);
    printf("========================================\n");
    
    return (total_errors > 0) ? ERROR : SUCCESS;
}

/*
 * Process a single assembly source file through all stages:
 * 1. Pre-assembler (macro expansion)
 * 2. First pass
 * 3. Second pass
 * 4. Output generation
 */
int process_file(const char *filename) {
    char source_file[256];
    char am_file[256];
    int result;
    SymbolTable symbol_table;
    MemoryImage memory;
    ExternalList externals;
    
    symbol_table_init(&symbol_table);
    externals.head = NULL;
    externals.count = 0;
    
    sprintf(source_file, "%s%s", filename, EXT_SOURCE);
    sprintf(am_file, "%s%s", filename, EXT_MACRO);
    
    printf("Stage 1: Macro expansion...\n");
    result = expand_macros(source_file, am_file);
    if (result == ERROR) {
        fprintf(stderr, "Error: Macro expansion failed for %s\n", source_file);
        symbol_table_free(&symbol_table);
        return ERROR;
    }
    printf("  → Generated %s\n", am_file);
    
    printf("Stage 2: First pass...\n");
    result = first_pass(am_file, &symbol_table, &memory);
    if (result == ERROR) {
        fprintf(stderr, "Error: First pass failed for %s\n", am_file);
        symbol_table_free(&symbol_table);
        return ERROR;
    }
    printf("  → Symbol table built\n");
    printf("  → IC = %d, DC = %d\n", memory.IC, memory.DC);
    
    symbol_table_print(&symbol_table);
    
    printf("Stage 3: Second pass...\n");
    result = execute_second_pass(am_file, &symbol_table, &memory, &externals);
    if (result == FALSE) {
        fprintf(stderr, "Error: Second pass failed for %s\n", am_file);
        symbol_table_free(&symbol_table);
        return ERROR;
    }
    printf("  → Encoding completed\n");
    
    printf("Stage 4: Generating output files...\n");
    create_object_file(filename, &memory);
    create_entries_file(filename, &symbol_table);
    create_externals_file(filename, &externals);
    printf("  → Output files generated\n");
    
    symbol_table_free(&symbol_table);
    
    return SUCCESS;
}