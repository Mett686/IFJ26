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
            //else error
            break;
        case ST_EOF:
            return TKN_EOF;
            //break;
        case ST_AT_MAIN: //split into 5 states?
            break;
        case ST_INDENTATION:
            while ((c = getc(stream)) == ' ') //count spaces
            break;
        case ST_ZERO_BEGINNING:
            if (c == '0') break; //stays
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
            if (isANum[c]) break; //stays
            else if (c == '_') state = ST_NZB_UNDERSCORE;
            else if (c == '.') state = ST_DOUBLE_DECIMAL_DOT;
            else return TKN_INT;
            break;
        case ST_NZB_UNDERSCORE:
            if (isANum[c]) state = ST_NONZERO_BEGINNING;
            //else error;
            break;
        case ST_DOUBLE_DECIMAL_DOT:
            if (isANum[c]) state = ST_DECIMAL_PART;
            //else error;
            break;
        case ST_DECIMAL_PART:
            if (isANum[c]) break; //stays 
            else if (c == '_') state = ST_DP_UNDERSCORE;
            else if (c == 'e' || c == 'E') state = ST_EXPONENT;
            else if (c == 'd' || c == 'D') state = ST_DOUBLE_SUFIX;
            else return TKN_DOUBLE;
            break;
        case ST_DP_UNDERSCORE:
            if (isANum[c]) state = ST_DECIMAL_PART;
            //else error;
            break;
        case ST_INTEGER_PART:
            if (isANum[c]) break; //stays
            else if (c == '_') state = ST_IP_UNDERSCORE;
            else if (c == '.') state = ST_DOUBLE_DECIMAL_DOT;
            else if (c == 'e' || c == 'E') state = ST_EXP_PREFIX;
            break;
        case ST_IP_UNDERSCORE:
            if (isANum[c]) state = ST_INTEGER_PART;
            //else error;
            break;
        case ST_EXP_PREFIX:
            if (isANum[c]) state = ST_EXPONENT;
            else if (c == '+' || c == '-') state = ST_EXP_SING;
            //else error;
            break;
        case ST_EXP_SING:
            if (isANum[c]) state = ST_EXPONENT;\
            //else error;
            break;
        case ST_EXPONENT:
            if (isANum[c]) break; //stays
            else if (c == '_') state = ST_EXP_UNDERSCORE;
            else if (c == 'd', c == 'D') state = ST_DOUBLE_SUFIX;
            else return TKN_DOUBLE;
            break;
        case ST_EXP_UNDERSCORE:
            if (isANum[c]) state = ST_EXPONENT;
            //else error;
            break;
        case ST_DOUBLE_SUFIX:
            return TKN_DOUBLE;
        case ST_MINUS:
            return TKN_MINUS;
        case ST_PLUS:
            return TKN_PLUS;
        case ST_DIVISION_SLASH:
            if (c == '*') state = ST_OPENING_BLOCK_COMMENT;
            else if (c == '/') state = ST_DOUBLE_SLASH_COMMENT;
            else return TKN_DIVISION_SLASH;
            break;
        case ST_TIMES:
            return TKN_TIMES;
        case ST_COMMA:
            return TKN_COMMA;
        case ST_COLON:
            return TKN_COLON;
        case ST_SEMICOLON:
            return TKN_SEMICOLON;
        case ST_LESS:
            if (c == '=') state = ST_LESS_EQUAL;
            else if (c == '-') state = ST_EXT_LESS_MINUS;
            else return TKN_LESS;
            break;
        case ST_LESS_EQUAL:
            return TKN_LESS_EQUAL;
        case ST_EXT_LESS_MINUS:
            return TKN_EXT_LESS_MINUS;
        case ST_GREATER:
            if (c == '=') state = ST_GREATER_EQUAL;
            else return TKN_GREATER;
            break;
        case ST_GREATER_EQUAL:
            return TKN_GREATER_EQUAL;
        case ST_EQUAL_SIGN:
            if (c == '=') state = ST_DOUBLE_EQUAL_SIGN;
            else if (c == '>') state = ST_EXT_EQUAL_GREATER;
            //else error;
            break;
        case ST_DOUBLE_EQUAL_SIGN:
            return TKN_EQUAL;
        case ST_EXT_EQUAL_GREATER:
            return TKN_EXT_EQUAL_GREATER;
        case ST_NOT:
            if (c == '=') state = ST_NOT_EQUAL;
            //else error;
            break;
        case ST_NOT_EQUAL:
            return TKN_NOT_EQUAL;
        case ST_OPENING_BLOCK_COMMENT: //požrat vše, while loop na vnořené
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
            return TKN_EXT_VERTICAL_LINE;
        case ST_EXT_UNDERSCORE:
            if (c == '_') state = ST_ID_UNDERSCORES;
            else return TKN_EXT_UNDERSCORE;
            break;
        case ST_ID_UNDERSCORES:
            if (c == '_') break;
            else if (isALetterOrANum[c]) state = ST_ID;
            break;
        case ST_ID:
            if (isALetterOrANum[c]) break;
            else return TKN_ID;
            break;
        case ST_OPENING_PARENTHESES:
            if (c == 10); //LF, TODO BEHAVES AS A WHITESPACE
            else if (c == ')') state = ST_UNIT;
            else return TKN_OPENING_PARENTHESES;
            break;
        case ST_CLOSING_PARENTHESES:
            return TKN_CLOSING_PARENTHESES;
        case ST_UNIT:
            return TKN_UNIT;
        case ST_OPENING_QUOTATION_MARK:
            if(c == '"') state = ST_TWO_QUOTATION_MARKS;
            else if (c == '\\') state = ST_IN_ESC_SEQ;
            else if (c == 10); //LF, TODO
            else state = state = ST_IN_STRING;
            break;
        case ST_TWO_QUOTATION_MARKS:
            if (c == '"') state = ST_MULTILINE_STRING;
            else return TKN_STRING; //empty
            break;
        case ST_MULTILINE_STRING:
            if (c == '"') state = ST_FIRST_CLOSING_QUATATION_MARK_IN_A_MULTILINE_STRING;
            //else break; //stays
            break;
        case ST_FIRST_CLOSING_QUATATION_MARK_IN_A_MULTILINE_STRING:
            if (c == '"') state = ST_SECOND_CLOSING_QUATATION_MARK_IN_A_MULTILINE_STRING;
            else state = ST_MULTILINE_STRING;
            break;
        case ST_SECOND_CLOSING_QUATATION_MARK_IN_A_MULTILINE_STRING:
            if (c == '"') state = ST_ENDING_A_MULTILINE_STRING;
            else state = ST_MULTILINE_STRING;
            break;
        case ST_ENDING_A_MULTILINE_STRING:
            return TKN_MULTILINE_STRING;
        case ST_IN_STRING:
            if (c == '"') state = ST_ENDING_STRING;
            else if (c == '\\') state = ST_IN_ESC_SEQ;
            else if (c == 10); //LF, TODO
            //else break; //stays
            break;
        case ST_IN_ESC_SEQ:
            if (isAnEscSeq[c]) state = ST_IN_STRING;
            //else LexErr1;
            break;
        case ST_ENDING_STRING:
            return TKN_STRING;
        }
    }
    return TKN_EOF;
}