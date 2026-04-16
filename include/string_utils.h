/*
 * string_utils.h
 * String manipulation utility functions
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

/*
 * Remove leading and trailing whitespace from a string
 * Modifies the string in place
 * Parameters:
 *   str - String to trim
 * Returns: Pointer to the trimmed string
 */
char* trim(char *str);

/*
 * Skip leading whitespace in a string
 * Does NOT modify the string
 * Parameters:
 *   str - String to process
 * Returns: Pointer to first non-whitespace character
 */
char* skip_whitespace(const char *str);

/*
 * Check if a line is empty or contains only whitespace/comment
 * Parameters:
 *   line - Line to check
 * Returns: TRUE if empty/comment, FALSE otherwise
 */
int is_empty_or_comment(const char *line);

/*
 * Check if a string is a valid label name
 * Valid: starts with letter, contains letters/digits, max 31 chars
 * Parameters:
 *   str - String to check
 * Returns: TRUE if valid label, FALSE otherwise
 */
int is_valid_label(const char *str);

/*
 * Check if a string is a reserved word (instruction/directive/register)
 * Parameters:
 *   str - String to check
 * Returns: TRUE if reserved, FALSE otherwise
 */
int is_reserved_word(const char *str);

/*
 * Extract the first word from a string
 * Parameters:
 *   str - Source string
 *   dest - Destination buffer
 *   max_len - Maximum length to copy
 * Returns: Pointer to the rest of the string after the word
 */
char* extract_word(char *str, char *dest, int max_len);

/*
 * Check if character is whitespace (space or tab)
 * Parameters:
 *   c - Character to check
 * Returns: TRUE if whitespace, FALSE otherwise
 */
int is_whitespace(char c);

/*
 * Copy a string safely (like strncpy but always null-terminates)
 * Parameters:
 *   dest - Destination buffer
 *   src - Source string
 *   n - Maximum number of characters to copy
 */
void safe_strcpy(char *dest, const char *src, int n);

#endif /* STRING_UTILS_H */