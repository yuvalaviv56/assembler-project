#ifndef OPERAND_ENCODER_H
#define OPERAND_ENCODER_H

/*
 * Encode an operand based on its addressing mode
 * Returns the encoded word
 */
int encode_operand(const char *operand, int mode);

/*
 * Encode an immediate value (#N)
 */
int encode_immediate(int value);

/*
 * Encode a register (rN)
 */
int encode_register(int reg_num);

#endif