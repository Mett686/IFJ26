// scanner_private.h
// Autor: Filip Maxa, FIT

#include <stdlib.h>
#include <stdio.h>
#define STRING_STARTING_CAPACITY 64
#define STACK_STARTING_CAPACITY 10

typedef enum {
    ST_START,
    ST_INDENTATION,
    ST_ZERO_BEGINNING, ST_ZB_UNDERSCORE,
    ST_NONZERO_BEGINNING, ST_NZB_UNDERSCORE,
    ST_DOUBLE_DECIMAL_DOT, ST_DECIMAL_PART, ST_INTEGER_PART, ST_DP_UNDERSCORE, ST_IP_UNDERSCORE, 
    ST_EXP_PREFIX, ST_EXP_SING, ST_EXPONENT, ST_EXP_UNDERSCORE,
    ST_DIVISION_SLASH,
    ST_LESS,
    ST_GREATER,
    ST_EQUAL_SIGN,
    ST_NOT,
    ST_OPENING_BLOCK_COMMENT,
    ST_EXT_UNDERSCORE,
    ST_ID,
    ST_OPENING_QUOTATION_MARK, ST_TWO_QUOTATION_MARKS, ST_MULTILINE_STRING, ST_FIRST_CLOSING_QUOTATION_MARK_IN_A_MULTILINE_STRING,
    ST_SECOND_CLOSING_QUOTATION_MARK_IN_A_MULTILINE_STRING, ST_THIRD_CLOSING_QUOTATION_MARK_IN_A_MULTILINE_STRING,
    ST_IN_STRING, ST_IN_ESC_SEQ
}  EState;

typedef struct {
    unsigned capacity;
    unsigned length; //bez NTC
    char* str;
} string_t;

void stringAppend(string_t* string, int c);

typedef struct {
    unsigned top;
    unsigned capacity;
    unsigned* stack;
} indentation_stack_t;

indentation_stack_t* push(indentation_stack_t* stack, unsigned spaces);