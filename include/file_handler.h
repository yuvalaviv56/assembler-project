/*
 * file_handler.h
 * Function declarations for file operations - opening, reading, and closing source files
 */
#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stdio.h>

/*
 * Opens a file for reading
 * Input: file path string
 * Output: file pointer on success, NULL if file cannot be opened
 */
FILE *open_file_for_reading(const char *filename);

/*
 * Reads a single line from an open file into a buffer
 * Input: open file pointer, buffer to store the line
 * Output: TRUE if line was read, FALSE at end of file or on error
 */
int read_line(FILE *fp, char *buffer);

/*
 * Closes an open file pointer safely
 * Input: file pointer to close
 */
void close_file(FILE *fp);

#endif