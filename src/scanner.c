#include "scanner_private.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>

token_t getToken(FILE* stream) {
    static bool lastTokenNL = false;
    enum EState state = ST_START;
    if(lastTokenNL)
        state = ST_INDENTATION;
    token_t token = {.type = TKN_NONE};
    string_t string = {.capacity = STRING_STARTING_CAPACITY, .length = 0, .str = NULL};
    string.str = (char*)malloc(STRING_STARTING_CAPACITY);
    if (string.str == NULL);
        abort(); //TODO
    bool inParentheses = false; //NL v závorkách se chová jako obyčejný whitespace
    int c;
    while ((c = getc(stream)) != EOF) 
    {
        if (c < 0 || 127 < c) //MIMO ASCII
            state = ST_LEX_ERROR_1;
        switch (state) //https://share.gemini.google/zIizBiReNRVX
        {
        case ST_START:
            string.str[0] = c;
            string.str[1] = '\0';
            string.length = 1;
            if(c == '0') state = ST_ZERO_BEGINNING;
            else if ('1' <= c && c <= '9') {
                token.intValue = (c - '0');
                state = ST_NONZERO_BEGINNING;
            }
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
            else if (('A' <= c && c <= 'Z')||('a' <= c && c <= 'z')) state = ST_ID;
            else if (c == '_') state = ST_EXT_UNDERSCORE;
            else if (c == ')') state = ST_CLOSING_PARENTHESES;
            else if (c == '(') state = ST_OPENING_PARENTHESES;
            else if (c == '"') {
                state = ST_OPENING_QUOTATION_MARK;
                string.length = 0;
                string.str[0] = '\0';
            }
            else if (c == '@') state = ST_AT_MAIN;
            else if (c == '10' && !inParentheses) {
                token.type = TKN_NEWLINE;
                lastTokenNL = true;
                free(string.str);
                return token;
            }
            else if (c <= 127) break; //zbylé ASCII ingoruji
            break;
        case ST_EOF:
            token.type = TKN_EOF;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_AT_MAIN:
            if (getc(stream) == 'm' &&
                getc(stream) == 'a' &&
                getc(stream) == 'i' &&
                getc(stream) == 'n'){
                    token.type = TKN_AT_MAIN;
                    lastTokenNL = false;
                    free(string.str);
                    return token;
                }
            else state = ST_LEX_ERROR_1;
            break;
        case ST_INDENTATION:
            unsigned spaces = 0;
            if(c = getc(stream) == 9) { //(tabulátor na začátku neprázdného řádku je lexikální chyba 1
                while (0 <= (c = getc(stream)) && c <= 32 || c == 127) {;}
                if(c != 10){
                    state = ST_LEX_ERROR_1;
                    break;
                }
                token.type = TKN_NEWLINE;
                lastTokenNL = true;
                free(string.str);
                return token;                    
            }
            if (c < 0 || 127 < c) { //MIMO ASCII
                state = ST_LEX_ERROR_1;
                break;
            } 
            while ((c = getc(stream)) == ' ') spaces++;
            if (c < 0 || 127 < c) { //MIMO ASCII
                state = ST_LEX_ERROR_1;
                break;
            } 
            ungetc(c, stream);
            token.type = TKN_INDENTATION;
            token.intValue = (int)spaces;
            lastTokenNL = false;
            free(string.str);
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
            else if ('1' <= c && c <= '9') { //TODO podporovat tento scénář?
                state = ST_INTEGER_PART;
                string.str[0] = c;
                string.str[1] = '\0';
                string.length = 1;
            }
            else {
                token.type = TKN_INT;
                token.intValue = 0;
                lastTokenNL = false;
                free(string.str);
                return token; //0
            }
            break;
        case ST_ZB_UNDERSCORE:
            if ('0' <= c && c <= '9') state = ST_ZERO_BEGINNING;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_NONZERO_BEGINNING:
            if ('0' <= c && c <= '9') {
                token.intValue = token.intValue * 10 + (c - '0');
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
                lastTokenNL = false;
                free(string.str);
                return token;
            }
            break;
        case ST_NZB_UNDERSCORE:
            if ('0' <= c && c <= '9') state = ST_NONZERO_BEGINNING;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_DOUBLE_DECIMAL_DOT:
            if ('0' <= c && c <= '9') {
                state = ST_DECIMAL_PART;
                stringAppend(&string, c);
            }
            else state = ST_LEX_ERROR_1;
            break;
        case ST_DECIMAL_PART:
            if ('0' <= c && c <= '9') {
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
                free(string.str);
                return token;
            }
            break;
        case ST_DP_UNDERSCORE:
            if ('0' <= c && c <= '9') state = ST_DECIMAL_PART;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_INTEGER_PART:
            if ('0' <= c && c <= '9') {
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
            if ('0' <= c && c <= '9') state = ST_INTEGER_PART;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_EXP_PREFIX:
            if ('0' <= c && c <= '9') {
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
            if ('1' <= c && c <= '9') {
                state = ST_EXPONENT;
                stringAppend(&string, c);
            }
            else state = ST_LEX_ERROR_1;
            break;
        case ST_EXPONENT:
            if ('0' <= c && c <= '9') {
                stringAppend(&string, c);
                break; //stays
            }
            else if (c == '_') state = ST_EXP_UNDERSCORE;
            else if (c == 'd', c == 'D') state = ST_DOUBLE_SUFIX;
            else {
                token.type = TKN_DOUBLE;
                //token.doubleValue = string TODO
                lastTokenNL = false;
                free(string.str);
                return token;
            }
            break;
        case ST_EXP_UNDERSCORE:
            if ('0' <= c && c <= '9') {
                state = ST_EXPONENT;
                stringAppend(&string, c);
            }
            else state = ST_LEX_ERROR_1;
            break;
        case ST_DOUBLE_SUFIX:
            token.type = TKN_DOUBLE;
            //token.doubleValue = string TODO
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_MINUS:
            token.type = TKN_MINUS;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_PLUS:
            token.type = TKN_PLUS;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_DIVISION_SLASH:
            if (c == '*') state = ST_OPENING_BLOCK_COMMENT;
            else if (c == '/') state = ST_DOUBLE_SLASH_COMMENT;
            else {
                token.type = TKN_DIVISION_SLASH;
                lastTokenNL = false;
                free(string.str);
                return token;
            }
            break;
        case ST_TIMES:
            token.type = TKN_TIMES;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_COMMA:
            token.type = TKN_COMMA;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_COLON:
            token.type = TKN_COLON;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_SEMICOLON:
            token.type = TKN_SEMICOLON;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_LESS:
            if (c == '=') state = ST_LESS_EQUAL;
            else if (c == '-') state = ST_EXT_LESS_MINUS;
            else {
                token.type = TKN_LESS;
                lastTokenNL = false;
                free(string.str);
                return token;
            }
            break;
        case ST_LESS_EQUAL:
            token.type = TKN_LESS_EQUAL;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_EXT_LESS_MINUS:
            token.type = TKN_EXT_LESS_MINUS;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_GREATER:
            if (c == '=') state = ST_GREATER_EQUAL;
            else {
                token.type = TKN_GREATER;
                lastTokenNL = false;
                free(string.str);
                return token;
            }
            break;
        case ST_GREATER_EQUAL:
            token.type = TKN_GREATER_EQUAL;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_EQUAL_SIGN:
            if (c == '=') state = ST_DOUBLE_EQUAL_SIGN;
            else if (c == '>') state = ST_EXT_EQUAL_GREATER;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_DOUBLE_EQUAL_SIGN:
            token.type = TKN_EQUAL;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_EXT_EQUAL_GREATER:
            token.type = TKN_EXT_EQUAL_GREATER;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_NOT:
            if (c == '=') state = ST_NOT_EQUAL;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_NOT_EQUAL:
            token.type = TKN_NOT_EQUAL;
            lastTokenNL = false;
            free(string.str);
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
                else if (c < 0 || 127 < c) { //MIMO ASCII
                    state = ST_LEX_ERROR_1;
                    break;
                } 
            }
            if(asNewLine){
                while ((c = getc(stream)) != 10 /*nl*/){
                    if(c == EOF){
                        state = ST_EOF;
                        break;
                    }
                    if(c < 0 || 32 < c && c != 127){
                        state = ST_LEX_ERROR_1;
                        break;
                    }
                }
                token.type = TKN_NEWLINE;
                lastTokenNL = true;
                free(string.str);
                return token;
            } 
            break;        
        case ST_DOUBLE_SLASH_COMMENT:
            if (c == '10') state = ST_INDENTATION; //komentář jako LF 
            break;
        case ST_EXT_VERTICAL_LINE:
            token.type = TKN_EXT_VERTICAL_LINE;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_EXT_UNDERSCORE:
            if (c == '_') {
                state = ST_ID_UNDERSCORES;
                stringAppend(&string, c);
            }
            else {
                token.type = TKN_EXT_UNDERSCORE;
                lastTokenNL = false;
                free(string.str);
                return token;
            }
            break;
        case ST_ID_UNDERSCORES:
            if (c == '_') {
                stringAppend(&string, c);
                break; //stays
            }
            else if (('0' <= c && c <= '9')||('A' <= c && 'Z' <= c)||('a' <= c && c <= 'z')) {
                state = ST_ID;
                stringAppend(&string, c);
            }
            break;
        case ST_ID:
            if (('0' <= c && c <= '9')||('A' <= c && 'Z' <= c)||('a' <= c && c <= 'z')) {
                stringAppend(&string, c);
                break; //stays
            } else {
                token.type = TKN_ID;
                token.string = string.str;
                lastTokenNL = false;
                if (c == '.') {
                    if( string.length == 5 &&
                        string.str[0] == 'i' &&
                        string.str[1] == 'f' &&
                        string.str[2] == 'j' &&
                        string.str[3] == '2' &&
                        string.str[4] == '6' &&
                        (c = getc(stream)) == '*'){ 
                            lastTokenNL = false;
                            token.type = TKN_IFJ26dot_asterisk;
                            free(string.str);
                            return token;
                    } else {
                        state = ST_LEX_ERROR_1;
                        break;
                    }
                }
                if (/*do, if*/ 2 <= string.length && string.length <= 9 /*extension*/) { //https://share.gemini.google/uzB0xNxAyMXs
                    switch (string.str[0]) //todo makra na zkraceni zapisu?
                    {
                        case 'c': //case, class 
                            if (string.length == 4 && string.str[1] == 'a' && string.str[2] == 's' && string.str[3] == 'e')
                                token.type = TKN_EXT_CASE;
                            else if (string.length == 5 && string.str[1] == 'l' && string.str[2] == 'a' && string.str[3] == 's' && string.str[4] == 's')
                                token.type = TKN_EXT_CLASS;
                            break;
                        case 'd': //def, do
                            if (string.length == 2 && string.str[1] == 'o')
                                token.type = TKN_DO;
                            else if (string.length == 3 && string.str[1] == 'e' && string.str[2] == 'f')
                                token.type = TKN_DEF;
                            break;
                        case 'D': //Double
                            if (string.length == 6 && string.str[1] == 'o' && string.str[2] == 'u' && string.str[3] == 'b' && string.str[4] == 'l' && string.str[5] == 'e')
                                token.type = TKN_DOUBLE;
                            break;
                        case 'e': //extension, else
                            if (string.length == 4 && string.str[1] == 'l' && string.str[2] == 's' && string.str[3] == 'e')
                                token.type = TKN_ELSE;
                            else if (string.length == 9 && string.str[1] == 'x' && string.str[2] == 't' && string.str[3] == 'e' && string.str[4] == 'n' && string.str[5] == 's' && string.str[6] == 'i' && string.str[7] == 'o' && string.str[8] == 'n')
                                token.type = TKN_EXT_EXTENSION;
                            break;
                        case 'f': //for
                            if (string.length == 3 && string.str[1] == 'o' && string.str[2] == 'r')
                                token.type == TKN_EXT_FOR;
                            break;
                        case 'i': //if, import
                            if (string.length == 2 && string.str[1] == 'f')
                                token.type = TKN_IF;
                            else if (string.length == 6 && string.str[1] == 'm' && string.str[2] == 'p' && string.str[3] == 'o' && string.str[4] == 'r' && string.str[5] == 't')
                                token.type = TKN_IMPORT;
                            break;
                        case 'I': //Int
                            if (string.length == 3 && string.str[1] == 'n' && string.str[2] == 't')
                                token.type = TKN_INT;
                            break;
                        case 'L': //List
                            if (string.length == 4 && string.str[1] == 'i' && string.str[2] == 's' && string.str[3] == 't')
                                token.type = TKN_EXT_LIST;
                            break;
                        case 'm': //match
                            if (string.length == 5 && string.str[1] == 'a' && string.str[2] == 't' && string.str[3] == 'c' && string.str[4] == 'h')
                                token.type = TKN_EXT_MATCH;
                            break;
                        case 'r': //return
                            if (string.length == 6 && string.str[1] == 'e' && string.str[2] == 't' && string.str[3] == 'u' && string.str[4] == 'r' && string.str[5] == 'n')
                                token.type = TKN_RETURN;
                            break;
                        case 'S': //String
                            if (string.length == 6 && string.str[1] == 't' && string.str[2] == 'r' && string.str[3] == 'i' && string.str[4] == 'n' && string.str[5] == 'g')
                                token.type = TKN_STRING;
                            break;
                        case 't': //then, type
                            if (string.length == 4){
                                if (string.str[1] == 'h' && string.str[2] == 'e' && string.str[3] == 'n')
                                    token.type = TKN_THEN;
                                else if (string.str[1] == 'y' && string.str[2] == 'p' && string.str[3] == 'e')
                                    token.type = TKN_EXT_TYPE;
                            }
                            break;
                        case 'U': //Unit
                            if (string.length == 4 && string.str[1] == 'n' && string.str[2] == 'i' && string.str[3] == 't')
                            break;
                        case 'v': //val, var
                            if (string.length == 3 && string.str[1] == 'a')
                                if(string.str[2] == 'l')
                                    token.type = TKN_VAL;
                                else if(string.str[2] == 'r')
                                    token.type = TKN_VAR;
                            break;
                        case 'w': //while
                            if (string.length == 5 && string.str[1] == 'h' && string.str[2] == 'i' && string.str[3] == 'l' && string.str[4] == 'e')
                            break;
                    default:
                        break;
                    }
                }
            }
            if (token.type != TKN_ID)
                free(string.str);
            return token; //free povinnost pro identifikátory se předává
        case ST_OPENING_PARENTHESES: //LF BEHAVES AS A WHITESPACE IN PARENTHESES
            inParentheses = true;
            if (c == ')') state = ST_UNIT;
            else {
                token.type = TKN_OPENING_PARENTHESES;
                lastTokenNL = false;
                free(string.str);
                return token;
            }
            break;
        case ST_CLOSING_PARENTHESES:
            inParentheses = false;
            token.type = TKN_CLOSING_PARENTHESES;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_UNIT:
            inParentheses = false;
            token.type = TKN_UNIT;
            lastTokenNL = false;
            free(string.str);
            return token;
        case ST_OPENING_QUOTATION_MARK:
            if(c == '"') state = ST_TWO_QUOTATION_MARKS;
            else if (c == '\\') {
                state = ST_IN_ESC_SEQ;
                string.str[0] = '\\';
                string.str[1] = '\0';
                string.length = 1;
            }
            else if (c == 10) state = ST_LEX_ERROR_1;
            else {
                state = ST_IN_STRING;
                string.str[0] = c;
                string.str[1] = '\0';
                string.length = 1;
            }
            break;
        case ST_TWO_QUOTATION_MARKS:
            if (c == '"') state = ST_MULTILINE_STRING;
            else {
                token.type = TKN_STRING;
                token.string = "\0";
                lastTokenNL = false;
                free(string.str);
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
            if (c == '"') {
                state = ST_ENDING_A_MULTILINE_STRING;
                string.length = string.length - 3;
                string.str[string.length] = '\0';
            }
            else {
                state = ST_MULTILINE_STRING;
                stringAppend(&string, c);
            }
            break;
        case ST_ENDING_A_MULTILINE_STRING:
            token.type = TKN_MULTILINE_STRING;
            token.string = string.str;
            lastTokenNL = false;
            return token; //free povinnost se předává
        case ST_IN_STRING:
            if (c == '"') state = ST_ENDING_STRING;
            else if (c == '\\') {
                state = ST_IN_ESC_SEQ;
                stringAppend(&string, c);
            }
            else if (c == 10){
                state = ST_LEX_ERROR_1;
                break;
            } 
            else stringAppend(&string, c);
            break; //else just stays
        case ST_IN_ESC_SEQ:
            if (c == '"' || c == '\\' || c == 'n' || c == 'r' || c == 't') state = ST_IN_STRING;
            else state = ST_LEX_ERROR_1;
            break;
        case ST_ENDING_STRING:
            token.type = TKN_STRING;
            lastTokenNL = false;
            return token; //free povinnost se předává
        case ST_LEX_ERROR_1:
            //TODO
            abort();
        }
    }
    token.type = TKN_EOF;
    free(string.str);
    lastTokenNL = false;
    return token;
}