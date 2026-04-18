#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stdio.h>

FILE *open_file_for_reading(const char *filename);
int read_line(FILE *fp, char *buffer);
void close_file(FILE *fp);

#endif