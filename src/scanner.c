#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>


token_t getToken(FILE* stream) {
    static bool lastTokenNL = false;
    enum EState state = ST_START;
    if(lastTokenNL)
        state = ST_INDENTATION;
    token_t token = {.type = TKN_NONE};
    stream = stdin;
    string_t string = {.capacity = STRING_STARTING_CAPACITY, .length = 0, .str = NULL};
    string.str = malloc(STRING_STARTING_CAPACITY);
    if (string.str == NULL);
        abort(); //TODO
    int c;
    bool inParentheses = false;
    while ((c = getc(stream)) != EOF) //ungetc(c, stream)
    {
        switch (state) //https://share.gemini.google/zIizBiReNRVX
        {
        case ST_START:
            string.length = 0;    
            string.str[string.length++] = c;
            string.str[string.length] = '\0';            
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
            else if (c == '@') state = ST_AT_MAIN;
            else if (c == '10' && !inParentheses) {
                token.type = TKN_NEWLINE;
                lastTokenNL = true;
                return token;
            }
            else if (c <= 127) break; //zbylé ASCII ingoruji
            else state = ST_LEX_ERROR_1; //neASCII error
            break;
        case ST_EOF:
            token.type = TKN_EOF;
            lastTokenNL = false;
            return token;
        case ST_AT_MAIN:
            if (getc(stream) == 'm' &&
                getc(stream) == 'a' &&
                getc(stream) == 'i' &&
                getc(stream) == 'n'){
                    token.type = TKN_AT_MAIN;
                    lastTokenNL = false;
                    return token;
                }
            else state = ST_LEX_ERROR_1;
            break;
        case ST_INDENTATION:
            unsigned spaces = 0;
            if(c = getc(stream) == 9) { //(tabulátor na začátku neprázdného řádku je lexikální chyba 1
                while (isAWhitespaceExceptNL[(c = getc(stream))]) {;}
                if(c != 10){
                    state = ST_LEX_ERROR_1;
                    break;
                }
                token.type = TKN_NEWLINE;
                lastTokenNL = true;
                return token;                    
            }
            while ((c = getc(stream)) == ' ') spaces++;
            ungetc(c, stream);
            token.type = TKN_INDENTATION;
            token.intValue = (int)spaces;
            lastTokenNL = false;
            return token; //i nula
            break;
        case ST_ZERO_BEGINNING:      
            if (c == '0') break; //stays in this state, string stays at 0
            else if (c == '_') state = ST_ZB_UNDERSCORE;
            else if (c == '.') {
                state = ST_DOUBLE_DECIMAL_DOT;
                string.str[string.length++] = c;
                string.str[string.length] = '\0';
            }
            else if (isAPosNum[c]) { //TODO podporovat tento scénář?
                state = ST_INTEGER_PART;
                string.str[0] = c;
                string.str[1] = '\0';
                string.length = 1;
            }
            else {
                token.type = TKN_INT;
                token.intValue = 0;
                lastTokenNL = false;
                return token; //0
            }
            break;
        case ST_ZB_UNDERSCORE:
            if (isANum[c]) state = ST_ZERO_BEGINNING;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_NONZERO_BEGINNING:
            if (isANum[c]) {
                stringAppend(&string, c);
                break; //stays
            } 
            else if (c == '_') state = ST_NZB_UNDERSCORE;
            else if (c == '.') {
                state = ST_DOUBLE_DECIMAL_DOT;
                stringAppend(&string, c);
            }    
            else {
                token.type = TKN_INT;
                //token.intValue = toInt(string); TODO
                lastTokenNL = false;
                return token;
            }
            break;
        case ST_NZB_UNDERSCORE:
            if (isANum[c]) state = ST_NONZERO_BEGINNING;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_DOUBLE_DECIMAL_DOT:
            if (isANum[c]) {
                state = ST_DECIMAL_PART;
                stringAppend(&string, c);
            }
            else state = ST_LEX_ERROR_1;
            break;
        case ST_DECIMAL_PART:
            if (isANum[c]) {
                stringAppend(&string, c);
                break; //stays 
            }
            else if (c == '_') state = ST_DP_UNDERSCORE;
            else if (c == 'e' || c == 'E') {
                state = ST_EXPONENT;
                stringAppend(&string, c);
            }
            else if (c == 'd' || c == 'D') state = ST_DOUBLE_SUFIX;
            else {
                token.type = TKN_DOUBLE;
                //token.doubleValue = string TODO
                lastTokenNL = false;
                return token;
            }
            break;
        case ST_DP_UNDERSCORE:
            if (isANum[c]) state = ST_DECIMAL_PART;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_INTEGER_PART:
            if (isANum[c]) {
                stringAppend(&string, c);
                break; //stays
            }
            else if (c == '_') state = ST_IP_UNDERSCORE;
            else if (c == '.') {
                state = ST_DOUBLE_DECIMAL_DOT;
                stringAppend(&string, c);
            }
            else if (c == 'e' || c == 'E') {
                state = ST_EXP_PREFIX;
                stringAppend(&string, c);
            }
            break;
        case ST_IP_UNDERSCORE:
            if (isANum[c]) state = ST_INTEGER_PART;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_EXP_PREFIX:
            if (isANum[c]) {
                state = ST_EXPONENT;
                stringAppend(&string, c);
            }
            else if (c == '+' || c == '-') {
                state = ST_EXP_SING;
                stringAppend(&string, c);
            }
            else state = ST_LEX_ERROR_1;
            break;
        case ST_EXP_SING:
            if (isANum[c]) {
                state = ST_EXPONENT;
                stringAppend(&string, c);
            }
            else state = ST_LEX_ERROR_1;
            break;
        case ST_EXPONENT:
            if (isANum[c]) {
                stringAppend(&string, c);
                break; //stays
            }
            else if (c == '_') state = ST_EXP_UNDERSCORE;
            else if (c == 'd', c == 'D') state = ST_DOUBLE_SUFIX;
            else {
                token.type = TKN_DOUBLE;
                //token.doubleValue = string TODO
                lastTokenNL = false;
                return token;
            }
            break;
        case ST_EXP_UNDERSCORE:
            if (isANum[c]) {
                state = ST_EXPONENT;
                stringAppend(&string, c);
            }
            else state = ST_LEX_ERROR_1;
            break;
        case ST_DOUBLE_SUFIX:
            token.type = TKN_DOUBLE;
            //token.doubleValue = string TODO
            lastTokenNL = false;
            return token;
        case ST_MINUS:
            token.type = TKN_MINUS;
            lastTokenNL = false;
            return token;
        case ST_PLUS:
            token.type = TKN_PLUS;
            lastTokenNL = false;
            return token;
        case ST_DIVISION_SLASH:
            if (c == '*') state = ST_OPENING_BLOCK_COMMENT;
            else if (c == '/') state = ST_DOUBLE_SLASH_COMMENT;
            else {
                token.type = TKN_DIVISION_SLASH;
                lastTokenNL = false;
                return token;
            }
            break;
        case ST_TIMES:
            token.type = TKN_TIMES;
            lastTokenNL = false;
            return token;
        case ST_COMMA:
            token.type = TKN_COMMA;
            lastTokenNL = false;
            return token;
        case ST_COLON:
            token.type = TKN_COLON;
            lastTokenNL = false;
            return token;
        case ST_SEMICOLON:
            token.type = TKN_SEMICOLON;
            lastTokenNL = false;
            return token;
        case ST_LESS:
            if (c == '=') state = ST_LESS_EQUAL;
            else if (c == '-') state = ST_EXT_LESS_MINUS;
            else {
                token.type = TKN_LESS;
                lastTokenNL = false;
                return token;
            }
            break;
        case ST_LESS_EQUAL:
            token.type = TKN_LESS_EQUAL;
            lastTokenNL = false;
            return token;
        case ST_EXT_LESS_MINUS:
            token.type = TKN_EXT_LESS_MINUS;
            lastTokenNL = false;
            return token;
        case ST_GREATER:
            if (c == '=') state = ST_GREATER_EQUAL;
            else {
                token.type = TKN_GREATER;
                lastTokenNL = false;
                return token;
            }
            break;
        case ST_GREATER_EQUAL:
            token.type = TKN_GREATER_EQUAL;
            lastTokenNL = false;
            return token;
        case ST_EQUAL_SIGN:
            if (c == '=') state = ST_DOUBLE_EQUAL_SIGN;
            else if (c == '>') state = ST_EXT_EQUAL_GREATER;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_DOUBLE_EQUAL_SIGN:
            token.type = TKN_EQUAL;
            lastTokenNL = false;
            return token;
        case ST_EXT_EQUAL_GREATER:
            token.type = TKN_EXT_EQUAL_GREATER;
            lastTokenNL = false;
            return token;
        case ST_NOT:
            if (c == '=') state = ST_NOT_EQUAL;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_NOT_EQUAL:
            token.type = TKN_NOT_EQUAL;
            lastTokenNL = false;
            return token;
        case ST_OPENING_BLOCK_COMMENT: //řeší i jiné stavy z FSM
            unsigned depthCount = 1;
            bool asNewLine = false;
            while (depthCount) {
                c = getc(stream);
                if (c == '10') asNewLine = true;
                else if (c == '/')
                    if ((c = getc(stream)) == '*')
                        depthCount++;
                    else if (c == '10') asNewLine = true; 
                else if (c == '*')
                    if ((c = getc(stream)) == '/')
                        depthCount--;
                    else if (c == '10') asNewLine = true;
            }
            if(asNewLine){
                token.type = TKN_NEWLINE;
                lastTokenNL = true;
                return token;
            } 
            break;        
        case ST_DOUBLE_SLASH_COMMENT:
            if (c == '10') state = ST_INDENTATION; //komentář jako LF 
            break;
        case ST_EXT_VERTICAL_LINE:
            token.type = TKN_EXT_VERTICAL_LINE;
            lastTokenNL = false;
            return token;
        case ST_EXT_UNDERSCORE:
            if (c == '_') {
                state = ST_ID_UNDERSCORES;
                stringAppend(&string, c);
            }
            else {
                token.type = TKN_EXT_UNDERSCORE;
                lastTokenNL = false;
                return token;
            }
            break;
        case ST_ID_UNDERSCORES:
            if (c == '_') {
                stringAppend(&string, c);
                break; //stays
            }
            else if (isALetterOrANum[c]) {
                state = ST_ID;
                stringAppend(&string, c);
            }
            break;
        case ST_ID:
            if (isALetterOrANum[c]) {
                stringAppend(&string, c);
                break; //stays
            }
            else {
                token.type = TKN_ID;
                token.string = string.str;
                lastTokenNL = false;
                return token;
            }
            break;
        case ST_OPENING_PARENTHESES: //LF BEHAVES AS A WHITESPACE IN PARENTHESES
            inParentheses = true;
            if (c == ')') state = ST_UNIT;
            else {
                token.type = TKN_OPENING_PARENTHESES;
                lastTokenNL = false;
                return token;
            }
            break;
        case ST_CLOSING_PARENTHESES:
            inParentheses = false;
            token.type = TKN_CLOSING_PARENTHESES;
            lastTokenNL = false;
            return token;
        case ST_UNIT:
            inParentheses = false;
            token.type = TKN_UNIT;
            lastTokenNL = false;
            return token;
        case ST_OPENING_QUOTATION_MARK:
            if(c == '"') state = ST_TWO_QUOTATION_MARKS;
            else if (c == '\\') state = ST_IN_ESC_SEQ;
            else if (c == 10) state = ST_LEX_ERROR_1;
            else state = state = ST_IN_STRING;
            break;
        case ST_TWO_QUOTATION_MARKS:
            if (c == '"') state = ST_MULTILINE_STRING;
            else {
                token.type = TKN_STRING;
                token.string = "\0";
                lastTokenNL = false;
                return token; //empty string
            }
            break;
        case ST_MULTILINE_STRING:
            if (c == '"') state = ST_FIRST_CLOSING_QUATATION_MARK_IN_A_MULTILINE_STRING;
            stringAppend(&string, c);
            break; //else just stays
        case ST_FIRST_CLOSING_QUATATION_MARK_IN_A_MULTILINE_STRING:
            if (c == '"') state = ST_SECOND_CLOSING_QUATATION_MARK_IN_A_MULTILINE_STRING;
            else state = ST_MULTILINE_STRING;
            stringAppend(&string, c);
            break;
        case ST_SECOND_CLOSING_QUATATION_MARK_IN_A_MULTILINE_STRING:
            if (c == '"') state = ST_ENDING_A_MULTILINE_STRING;
            else state = ST_MULTILINE_STRING;
            break;
        case ST_ENDING_A_MULTILINE_STRING:
            token.type = TKN_MULTILINE_STRING;
            lastTokenNL = false;
            return token;
        case ST_IN_STRING:
            if (c == '"') state = ST_ENDING_STRING;
            else if (c == '\\') state = ST_IN_ESC_SEQ;
            else if (c == 10) state = ST_LEX_ERROR_1;
            break; //else just stays
        case ST_IN_ESC_SEQ:
            if (isAnEscSeq[c]) state = ST_IN_STRING;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_ENDING_STRING:
            token.type = TKN_STRING;
            lastTokenNL = false;
            return token;
        case ST_LEX_ERROR_1:
            //TODO
            //TODO všechny možnosti chyb, minimálně chybí tabulátor a pak nic   
        }
    }
    free(string.str); //TODO
    token.type = TKN_EOF;
    lastTokenNL = false;
    return token;
}