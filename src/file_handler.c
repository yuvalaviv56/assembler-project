/*
 * file_handler.c
 * Handles file operations - opening, reading, and closing source files
 */
#include <stdio.h>
#include <string.h>
#include "file_handler.h"
#include "globals.h"

/*
 * Opens a file for reading
 * Input: file path string
 * Output: file pointer on success, NULL if file cannot be opened
 */
FILE *open_file_for_reading(const char *filename) {
    FILE *fp;   /* file pointer for the opened file */
    
    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    return fp;
}

/*
 * Reads a single line from an open file into a buffer
 * Input: open file pointer, buffer to store the line
 * Output: TRUE if line was read, FALSE at end of file or on error
 */
int read_line(FILE *fp, char *buffer) {
    if (fgets(buffer, MAX_LINE_LENGTH, fp) != NULL) {
        return TRUE;
    }
    return FALSE;
}

/*
 * Closes an open file pointer safely
 * Input: file pointer to close
 */
void close_file(FILE *fp) {
    if (fp != NULL) {
        fclose(fp);
    }
}