/*
 * macro.h
 * Macro expansion function declarations
 */

#ifndef MACRO_H
#define MACRO_H

/*
 * The main pre-assembler function - expands all macros in source file
 * Input: path to source .as file, path to output .am file
 * Output: SUCCESS if expansion completed, ERROR if any issues found
 */
int expand_macros(const char *source, const char *output);

#endif /* MACRO_H */