// scanner.h
// Autor: Filip Maxa, FIT

#ifndef SCANNER_H
#define SCANNER_H

#include <stdbool.h>
#include <stdio.h>

typedef enum { //TODO add newline
    TKN_NONE = 0,
    TKN_ID,
    TKN_ASSING,
    TKN_AT_MAIN, TKN_EOF,
    TKN_INT, TKN_INT_KEYWORD, TKN_DOUBLE, TKN_DOUBLE_KEYWORD, TKN_UNIT, TKN_UNIT_KEYWORD, 
    TKN_STRING_KEYWORD, TKN_STRING, TKN_MULTILINE_STRING,
    TKN_VAL, TKN_VAR,
    TKN_DEF,
    TKN_DO, TKN_WHILE,
    TKN_IF, TKN_ELSE, TKN_THEN,
    TKN_IMPORT,
    TKN_IFJ26dot_asterisk,
    TKN_RETURN,
    TKN_NEWLINE,
    TKN_INDENTATION, TKN_DEDENTATION,
    TKN_MINUS, TKN_PLUS,
    TKN_COMMA, TKN_COLON, TKN_SEMICOLON,
    TKN_DIVISION_SLASH, TKN_TIMES,
    TKN_NOT_EQUAL, TKN_EQUAL,
    TKN_LESS, TKN_LESS_EQUAL, TKN_GREATER, TKN_GREATER_EQUAL,
    TKN_OPENING_PARENTHESES, TKN_CLOSING_PARENTHESES,
    TKN_OPENING_SQUARE_PARENTHESES, TKN_CLOSING_SQUARE_PARENTHESES,
    /*TKN_EXT_BOOLEAN, TODO*/ TKN_EXT_CASE, TKN_EXT_CLASS, TKN_EXT_EXTENSION, TKN_EXT_FOR, TKN_EXT_LIST, 
    TKN_EXT_MATCH, TKN_EXT_TYPE, TKN_EXT_EQUAL_GREATER, TKN_EXT_VERTICAL_LINE, TKN_EXT_LESS_MINUS, TKN_EXT_UNDERSCORE,
    TKN_BOOLEAN, TKN_DOT
}  ETokenType;

typedef struct {
    ETokenType type;
    union {
        long long int intValue;
        double doubleValue;
        char* string;
    };
} token_t;

/** *
 * Function returns one token. In case of a token type TKN_ID, TKN_STRING or TKN_MULTILINE_STRING the caller is responsible for freeing token_t.string.
 * TODO Or ends the program with lexical error.
 * @param stream function takes one paramatre, that is FILE* where the input shall be found. Caller is responsible for it being a valid ptr. 
 */
token_t getToken(FILE* stream);

//https://share.gemini.google/2iSR6yQzxtX5 END OF LOOK–UP ARRAYS

#endif
