/*
 * operand_encoder.c
 * Translates operands into their corresponding 12-bit machine code representation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operand_encoder.h"
#include "globals.h"

/* Translates an immediate value into a 12-bit word
 * Input: int value
 * Output: same value in 12-bit format */
int encode_immediate(int value) {
    return value & 0xFFF;
}

/* Translates a register number into a 12-bit word with the corresponding bit set to 1
 * Input: register number (0-7)
 * Output: 12-bit word with bit N set for register rN, all zeros if invalid */
int encode_register(int reg_num) {
    if (reg_num < 0 || reg_num > 7) {
        return 0;
    }
    return (1 << reg_num);
}

/* Translates an operand string based on its addressing mode
 * Input: operand string, addressing mode (0=immediate, 3=register)
 * Output: corresponding 12-bit word, 0 for unsupported modes */
int encode_operand(const char *operand, int mode) {
    int value;      /* parsed immediate value */
    int reg_num;    /* parsed register number */
    
    /* safety check - null operand returns 0 */
    if (operand == NULL) {
        return 0;
    }
    
    /* immediate mode - skip '#' prefix and convert to int */
    if (mode == 0) {
        value = atoi(operand + 1);
        return encode_immediate(value);
    }
    
    /* register mode - extract register number from second character */
    if (mode == 3) {
        reg_num = operand[1] - '0';
        return encode_register(reg_num);
    }
    
    return 0;       /* direct and relative modes handled in second pass */
}