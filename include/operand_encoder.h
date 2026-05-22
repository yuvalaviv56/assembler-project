/*
 * operand_encoder.h
 * Function declarations for operand translation into 12-bit machine code words
 */
#ifndef OPERAND_ENCODER_H
#define OPERAND_ENCODER_H

/*
 * Translates an operand string based on its addressing mode
 * Input: operand string, addressing mode (0=immediate, 3=register)
 * Output: corresponding 12-bit word, 0 for unsupported modes
 */
int encode_operand(const char *operand, int mode);

/* 
 * Translates an immediate value into a 12-bit word
 * Input: int value
 * Output: same value in 12-bit format 
 */
int encode_immediate(int value);

/*
 * Translates a register number into a 12-bit word with the corresponding bit set
 * Input: register number (0-7)
 * Output: 12-bit word with bit N set for register rN, all zeros if invalid
 */
int encode_register(int reg_num);

#endif