/*
 * macro.h
 * Macro expansion function declarations
 */

#ifndef MACRO_H
#define MACRO_H

/*
 * Expands macros from source to output file
 * Parameters:
 *   source - Input filename
 *   output - Output filename
 * Returns: SUCCESS or ERROR
 */
int expand_macros(const char *source, const char *output);

#endif /* MACRO_H */