#include <stdbool.h>

enum EState {

};

enum ETokenType{
    ID = 0,
    AT_MAIN, EOF,
    INT, DOUBLE, UNIT, STRING, MULTILINE_STRING,
    VAL, VAR,
    DEF,
    DO, WHILE,
    IF, ELSE, THEN,
    IMPORT,
    RETURN,
    INDENTATION,
    MINUS, PLUS,
    COMMA, COLON, SEMICOLON,
    DIVISION_SLASH, TIMES,
    NOT_EQUAL, EQUAL,
    LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    OPENING_PARANTHESES, CLOSING_PARANTHESES ,
    EXT_BOOLEAN, EXT_CASE, EXT_CLASS, EXT_EXTENSION, EXT_FOR, EXT_LIST, EXT_MATCH, EXT_TYPE, EXT_EQUAL_GREATER, EXT_VERTICAL_LINE, EXT_LESS_MINUS, EXT_UNDERSCORE
};

const bool isANum[128];
const bool isAPosNum[128];
//const bool isAZero[128];
const bool isALetter[128];
const bool isALetterOrANum[128];
const bool isAnEscSeq[128];
const bool isAWhitespace[128];
const char AT_MAIN[] = "@main";
