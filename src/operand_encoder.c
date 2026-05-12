#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operand_encoder.h"
#include "globals.h"

int encode_immediate(int value) {
    return value & 0xFFF;
}

int encode_register(int reg_num) {
    if (reg_num < 0 || reg_num > 7) {
        return 0;
    }
    return (1 << reg_num);
}

int encode_operand(const char *operand, int mode) {
    int value;
    int reg_num;
    
    if (operand == NULL) {
        return 0;
    }
    
    if (mode == 0) {
        value = atoi(operand + 1);
        return encode_immediate(value);
    }
    
    if (mode == 3) {
        reg_num = operand[1] - '0';
        return encode_register(reg_num);
    }
    
    return 0;
}