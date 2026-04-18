#include <stdio.h>
#include <string.h>
#include "file_handler.h"
#include "globals.h"

FILE *open_file_for_reading(const char *filename) {
    FILE *fp;
    
    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    return fp;
}

int read_line(FILE *fp, char *buffer) {
    if (fgets(buffer, MAX_LINE_LENGTH, fp) != NULL) {
        return TRUE;
    }
    return FALSE;
}

void close_file(FILE *fp) {
    if (fp != NULL) {
        fclose(fp);
    }
}