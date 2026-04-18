/*
 * string_utils.c
 * Implementation of string utility functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string_utils.h"
#include "constants.h"
#include "globals.h"

/* Helper: Check if character is whitespace */
int is_whitespace(char c) {
    return (c == ' ' || c == '\t');
}

/* Trim leading and trailing whitespace */
char* trim(char *str) {
    char *start;
    char *end;
    
    if (str == NULL || *str == '\0') {
        return str;
    }
    
    /* Trim leading whitespace */
    start = str;
    while (is_whitespace(*start) || *start == '\n' || *start == '\r') {
        start++;
    }
    
    /* If string is all whitespace */
    if (*start == '\0') {
        *str = '\0';
        return str;
    }
    
    /* Trim trailing whitespace */
    end = start + strlen(start) - 1;
    while (end > start && (is_whitespace(*end) || *end == '\n' || *end == '\r')) {
        end--;
    }
    
    /* Null terminate */
    *(end + 1) = '\0';
    
    /* Move trimmed string to beginning if needed */
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    return str;
}

/* Skip leading whitespace (non-destructive) */
char* skip_whitespace(const char *str) {
    if (str == NULL) {
        return NULL;
    }
    
    while (is_whitespace(*str)) {
        str++;
    }
    
    return (char*)str;
}

/* Check if line is empty or comment */
int is_empty_or_comment(const char *line) {
    const char *p;
    
    if (line == NULL) {
        return TRUE;
    }
    
    p = skip_whitespace(line);
    
    /* Empty line or comment (handle \r\n too) */
    return (*p == '\0' || *p == '\n' || *p == '\r' || *p == COMMENT_CHAR);
}

/* Check if string is a valid label name */
int is_valid_label(const char *str) {
    int len;
    int i;
    
    if (str == NULL || *str == '\0') {
        return FALSE;
    }
    
    /* Must start with a letter */
    if (!isalpha(*str)) {
        return FALSE;
    }
    
    /* Check length */
    len = strlen(str);
    if (len > MAX_LABEL_LENGTH) {
        return FALSE;
    }
    
    /* Check all characters are alphanumeric or underscore */
    for (i = 0; i < len; i++) {
        if (!isalnum(str[i]) && str[i] != '_') {
            return FALSE;
        }
    }
    
    /* Check if it's a reserved word */
    if (is_reserved_word(str)) {
        return FALSE;
    }
    
    return TRUE;
}

/* Check if string is a reserved word */
int is_reserved_word(const char *str) {
    /* Reserved instructions */
    static const char *instructions[] = {
        "mov", "cmp", "add", "sub", "lea",
        "clr", "not", "inc", "dec",
        "jmp", "bne", "jsr", "red", "prn",
        "rts", "stop",
        NULL
    };
    
    /* Reserved directives */
    static const char *directives[] = {
        ".data", ".string", ".entry", ".extern",
        NULL
    };
    
    /* Reserved macro keywords */
    static const char *macro_keywords[] = {
        "mcro", "mcroend",
        NULL
    };
    
    /* Reserved registers */
    static const char *registers[] = {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        NULL
    };
    
    int i;
    
    if (str == NULL) {
        return FALSE;
    }
    
    /* Check against all reserved word lists */
    for (i = 0; instructions[i] != NULL; i++) {
        if (strcmp(str, instructions[i]) == 0) {
            return TRUE;
        }
    }
    
    for (i = 0; directives[i] != NULL; i++) {
        if (strcmp(str, directives[i]) == 0) {
            return TRUE;
        }
    }
    
    for (i = 0; macro_keywords[i] != NULL; i++) {
        if (strcmp(str, macro_keywords[i]) == 0) {
            return TRUE;
        }
    }
    
    for (i = 0; registers[i] != NULL; i++) {
        if (strcmp(str, registers[i]) == 0) {
            return TRUE;
        }
    }
    
    return FALSE;
}

/* Extract the first word from a string */
char* extract_word(char *str, char *dest, int max_len) {
    int i;
    char *start;
    
    if (str == NULL || dest == NULL) {
        return NULL;
    }
    
    /* Skip leading whitespace */
    start = skip_whitespace(str);
    
    /* Extract word (stop at whitespace or special chars) */
    i = 0;
    while (start[i] != '\0' && 
        !is_whitespace(start[i]) && 
        start[i] != '\n' &&
        start[i] != '\r' &&  /* ← Add this line */
        start[i] != LABEL_END &&
        start[i] != OPERAND_SEPARATOR &&
        i < max_len - 1) {
        dest[i] = start[i];
        i++;
    }
    
    dest[i] = '\0';
    
    /* Return pointer to rest of string */
    return start + i;
}

/* Safe string copy (always null-terminates) */
void safe_strcpy(char *dest, const char *src, int n) {
    if (dest == NULL || src == NULL || n <= 0) {
        return;
    }
    
    strncpy(dest, src, n - 1);
    dest[n - 1] = '\0';
}