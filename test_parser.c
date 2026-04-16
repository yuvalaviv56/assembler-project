/*
 * test_parser.c
 * Test program for parser implementation
 * 
 * Compile: gcc -Wall -ansi -pedantic -Iinclude -o test_parser \
 *          test_parser.c src/parser.c src/string_utils.c
 */

#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "constants.h"

void test_line_type_detection(void);
void test_label_extraction(void);
void test_operand_parsing(void);
void test_addressing_modes(void);
void test_directive_parsing(void);

int main(void) {
    printf("========================================\n");
    printf("Parser Unit Tests\n");
    printf("========================================\n\n");
    
    test_line_type_detection();
    test_label_extraction();
    test_operand_parsing();
    test_addressing_modes();
    test_directive_parsing();
    
    printf("========================================\n");
    printf("All parser tests completed!\n");
    printf("========================================\n");
    
    return 0;
}

void test_line_type_detection(void) {
    printf("Test 1: Line Type Detection\n");
    printf("----------------------------\n");
    
    if (parse_line_type("; comment") == LINE_EMPTY) {
        printf("✓ Detected comment as empty\n");
    }
    
    if (parse_line_type("") == LINE_EMPTY) {
        printf("✓ Detected empty line\n");
    }
    
    if (parse_line_type("MAIN: mov #5, r1") == LINE_INSTRUCTION) {
        printf("✓ Detected instruction with label\n");
    }
    
    if (parse_line_type("add r1, r2") == LINE_INSTRUCTION) {
        printf("✓ Detected instruction without label\n");
    }
    
    if (parse_line_type(".data 5, -10") == LINE_DIRECTIVE) {
        printf("✓ Detected .data directive\n");
    }
    
    if (parse_line_type("STR: .string \"hello\"") == LINE_DIRECTIVE) {
        printf("✓ Detected .string directive with label\n");
    }
    
    printf("\n");
}

void test_label_extraction(void) {
    char line1[] = "MAIN: mov #5, r1";
    char line2[] = "add r1, r2";
    char line3[] = "LOOP:  jmp %LOOP";
    char label[MAX_LABEL_LENGTH + 1];
    char *rest;
    
    printf("Test 2: Label Extraction\n");
    printf("-------------------------\n");
    
    rest = parse_label(line1, label, MAX_LABEL_LENGTH + 1);
    if (strcmp(label, "MAIN") == 0) {
        printf("✓ Extracted label 'MAIN'\n");
        printf("  Rest of line: '%s'\n", rest);
    }
    
    rest = parse_label(line2, label, MAX_LABEL_LENGTH + 1);
    if (label[0] == '\0') {
        printf("✓ No label found in line without label\n");
    }
    
    rest = parse_label(line3, label, MAX_LABEL_LENGTH + 1);
    if (strcmp(label, "LOOP") == 0) {
        printf("✓ Extracted label 'LOOP' (with extra spaces)\n");
    }
    
    printf("\n");
}

void test_operand_parsing(void) {
    char operands1[] = "#5, r1";
    char operands2[] = "r3";
    char operands3[] = "LABEL, r2";
    char source[50], dest[50];
    int count;
    
    printf("Test 3: Operand Parsing\n");
    printf("------------------------\n");
    
    count = parse_operands(operands1, source, dest, 50);
    if (count == 2 && strcmp(source, "#5") == 0 && strcmp(dest, "r1") == 0) {
        printf("✓ Parsed two operands: '%s', '%s'\n", source, dest);
    }
    
    count = parse_operands(operands2, source, dest, 50);
    if (count == 1 && strcmp(dest, "r3") == 0) {
        printf("✓ Parsed single operand: '%s'\n", dest);
    }
    
    count = parse_operands(operands3, source, dest, 50);
    if (count == 2 && strcmp(source, "LABEL") == 0 && strcmp(dest, "r2") == 0) {
        printf("✓ Parsed label and register: '%s', '%s'\n", source, dest);
    }
    
    printf("\n");
}

void test_addressing_modes(void) {
    Operand op;
    
    printf("Test 4: Addressing Mode Detection\n");
    printf("----------------------------------\n");
    
    if (identify_addressing_mode("#5") == MODE_IMMEDIATE) {
        printf("✓ Detected immediate mode: #5\n");
    }
    
    if (identify_addressing_mode("r1") == MODE_REGISTER) {
        printf("✓ Detected register mode: r1\n");
    }
    
    if (identify_addressing_mode("LABEL") == MODE_DIRECT) {
        printf("✓ Detected direct mode: LABEL\n");
    }
    
    if (identify_addressing_mode("%LOOP") == MODE_RELATIVE) {
        printf("✓ Detected relative mode: %%LOOP\n");
    }
    
    /* Test parse_operand */
    if (parse_operand("#-10", &op) && op.mode == MODE_IMMEDIATE && op.value == -10) {
        printf("✓ Parsed immediate operand: #-10 → value=%d\n", op.value);
    }
    
    if (parse_operand("r5", &op) && op.mode == MODE_REGISTER && op.value == 5) {
        printf("✓ Parsed register operand: r5 → reg=%d\n", op.value);
    }
    
    if (parse_operand("MAIN", &op) && op.mode == MODE_DIRECT && strcmp(op.symbol, "MAIN") == 0) {
        printf("✓ Parsed direct operand: MAIN → symbol='%s'\n", op.symbol);
    }
    
    printf("\n");
}

void test_directive_parsing(void) {
    char line1[] = ".data 5, -10, 100";
    char line2[] = ".string \"hello world\"";
    char directive[50], params[200];
    
    printf("Test 5: Directive Parsing\n");
    printf("--------------------------\n");
    
    if (parse_directive(line1, directive, params, 200)) {
        printf("✓ Parsed directive: '%s'\n", directive);
        printf("  Parameters: '%s'\n", params);
    }
    
    if (parse_directive(line2, directive, params, 200)) {
        printf("✓ Parsed directive: '%s'\n", directive);
        printf("  Parameters: '%s'\n", params);
    }
    
    if (is_valid_directive(".data")) {
        printf("✓ Validated .data as directive\n");
    }
    
    if (is_valid_directive(".string")) {
        printf("✓ Validated .string as directive\n");
    }
    
    if (!is_valid_directive(".invalid")) {
        printf("✓ Rejected invalid directive\n");
    }
    
    printf("\n");
}