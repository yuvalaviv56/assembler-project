#include <stdio.h>
#include "macro.h"

int main() {
    int result = expand_macros("tests/test1.as", "tests/test1.am");
    printf("expand_macros returned: %d\n", result);
    return 0;
}
