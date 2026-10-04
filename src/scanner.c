#include "scanner.h"
#include <stdio.h>


int getToken() {
    enum ETokenType tokenType = TKN_NONE;
    enum EState state = ST_START;
    FILE* stream = stdin;
    int c;
    while ((c = getc(stream)) != EOF) //ungetc(c, stream)
    {
        switch (state) //https://share.gemini.google/zIizBiReNRVX
        {
        case ST_START:
            //TODO main, whitespaces
            if(c == '0') state = ST_ZERO_BEGINNING;
            else if (isAPosNum[c]) state = ST_NONZERO_BEGINNING;
            else if (c == '-') state = ST_MINUS;
            else if (c == '+') state = ST_PLUS;
            else if (c == '/') state = ST_DIVISION_SLASH;
            else if (c == '*') state = ST_TIMES;
            else if (c == ',') state = ST_COMMA;
            else if (c == ':') state = ST_COLON;
            else if (c == ';') state = ST_SEMICOLON;
            else if (c == '!') state = ST_NOT;
            else if (c == '=') state = ST_EQUAL_SIGN;
            else if (c == '>') state = ST_GREATER;
            else if (c == '<') state = ST_LESS;
            else if (isALetter[c]) state = ST_ID;
            else if (c == '_') state = ST_EXT_UNDERSCORE;
            else if (c == ')') state = ST_CLOSING_PARENTHESES;
            else if (c == '(') state = ST_OPENING_PARENTHESES;
            else if (c == '"') state = ST_OPENING_QUOTATION_MARK;
            break;
        case ST_EOF:
            return TKN_EOF;
            //break;
        case ST_AT_MAIN:
            break;
        case ST_INDENTATION:
            break;
        case ST_ZERO_BEGINNING:
            if (c == '0') state = ST_ZERO_BEGINNING;
            else if (c == '_') state = ST_ZB_UNDERSCORE;
            else if (c == '.') state = ST_DOUBLE_DECIMAL_DOT;
            else if (isAPosNum[c]) state = ST_INTEGER_PART;
            else return TKN_INT; //0
            break;
        case ST_ZB_UNDERSCORE:
            if (isANum[c]) state = ST_ZERO_BEGINNING;
            //else error?;
            break;
        case ST_NONZERO_BEGINNING:
            break;
        case ST_NZB_UNDERSCORE:
            break;
        case ST_DOUBLE_DECIMAL_DOT:
            break;
        case ST_DECIMAL_PART:
            break;
        case ST_INTEGER_PART:
            break;
        case ST_DP_UNDERSCORE:
            break;
        case ST_IP_UNDERSCORE:
            break;
        case ST_EXP_PREFIX:
            break;
        case ST_EXP_SING:
            break;
        case ST_EXPONENT:
            break;
        case ST_EXP_UNDERSCORE:
            break;
        case ST_DOUBLE_SUFIX:
            break;
        case ST_MINUS:
            break;
        case ST_PLUS:
            break;
        case ST_DIVISION_SLASH:
            break;
        case ST_TIMES:
            break;
        case ST_COMMA:
            break;
        case ST_COLON:
            break;
        case ST_SEMICOLON:
            break;
        case ST_LESS:
            break;
        case ST_LESS_EQUAL:
            break;
        case ST_EXT_LESS_MINUS:
            break;
        case ST_GREATER:
            break;
        case ST_GREATER_EQUAL:
            break;
        case ST_EQUAL_SIGN:
            break;
        case ST_DOUBLE_EQUAL_SIGN:
            break;
        case ST_EXT_EQUAL_GREATER:
            break;
        case ST_NOT:
            break;
        case ST_NOT_EQUAL:
            break;
        case ST_OPENING_BLOCK_COMMENT:
            break;
        case ST_ASTERISK_IN_BLOCK_COMMENT:
            break;
        case ST_ENDING_BLOCK_COMMENT:
            break;
        case ST_BLOCK_COMMENT_WITH_LF:
            break;
        case ST_ASTERISK_BLOCK_COMMENT_WITH_LF:
            break;
        case ST_ENDING_BLOCK_COMMENT_WITH_LF:
            break;
        case ST_DOUBLE_SLASH_COMMENT:
            break;
        case ST_EXT_VERTICAL_LINE:
            break;
        case ST_EXT_UNDERSCORE:
            break;
        case ST_ID_UNDERSCORES:
            break;
        case ST_ID:
            break;
        case ST_OPENING_PARENTHESES:
            break;
        case ST_CLOSING_PARENTHESES:
            break;
        case ST_UNIT:
            break;
        case ST_OPENING_QUOTATION_MARK:
            break;
        case ST_TWO_QUOTATION_MARKS:
            break;
        case ST_MULTILINE_STRING:
            break;
        case ST_FIRST_CLOSING_QUATATION_MARK_IN_A_MULTILINE_STRING:
            break;
        case ST_SECOND_CLOSING_QUATATION_MARK_IN_A_MULTILINE_STRING:
            break;
        case ST_ENDING_A_MULTILINE_STRING:
            break;
        case ST_IN_STRING:
            break;
        case ST_IN_ESC_SEQ:
            break;
        case ST_ENDING_STRING:
            break;
        default:
            break;
        }
    }
    return TKN_EOF;
}