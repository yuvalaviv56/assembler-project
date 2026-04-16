#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>

/* הגדרת bool */
typedef enum {
    FALSE = 0,
    TRUE = 1
} bool;

/* קבועים */
#define MAX_LINE_LENGTH 81
#define MAX_LABEL_LENGTH 31
#define MEMORY_SIZE 4096
#define INITIAL_IC 100
#define INITIAL_DC 0

#endif