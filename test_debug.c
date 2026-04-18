#include <stdio.h>
#include <string.h>

char* trim(char *str) {
    char *start, *end;
    if (!str || !*str) return str;
    
    start = str;
    while (*start == ' ' || *start == '\t') start++;
    
    if (*start == '\0') {
        *str = '\0';
        return str;
    }
    
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    
    *(end + 1) = '\0';
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    return str;
}

int main() {
    char operands[] = " \n";
    printf("Before trim: '%s' (len=%lu)\n", operands, strlen(operands));
    trim(operands);
    printf("After trim: '%s' (len=%lu)\n", operands, strlen(operands));
    printf("Is empty: %s\n", *operands == '\0' ? "YES" : "NO");
    return 0;
}
