// scanner.c
// Autor: Filip Maxa, FIT

#include "scanner_private.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1
#define STR_CMP(i, c) (string.str[i] == (c))
#define TOKEN_FOUND_RETURN_TO_START(tkn) {foundToken = true; state = ST_START; token.type = tkn;}
#define COMPARE_WITH_KEYWORD(keyword, tkn) if(!strcmp(string.str, #keyword)) TOKEN_FOUND_RETURN_TO_START(tkn) 
#define EOF_AND_OUT_OF_ASCII_CHECK if (c == EOF) {foundToken = true; token.type = TKN_EOF; break;}  else if (c < 0 || 127 < c) {free(string.str); exit(1);} //LEX ERR 1 //MIMO ASCII KROMĚ EOF

indentation_stack_t indentationStack = {.top = 0, .capacity = (unsigned)STACK_STARTING_CAPACITY, .stack = NULL};

token_t getToken(FILE* stream) {
    bool foundToken = false;
    static EState state = ST_START;
    token_t token = {.type = TKN_NONE};
    if(indentationStack.stack == NULL){
        indentationStack.stack = (unsigned*)malloc(STACK_STARTING_CAPACITY);
        if(indentationStack.stack == NULL)
            exit(-1);
        indentationStack.stack[0] = 0;
    }
    static unsigned toDedent = 0;
    string_t string = {.capacity = (unsigned)STRING_STARTING_CAPACITY, .length = 0, .str = NULL};
    string.str = (char*)malloc(STRING_STARTING_CAPACITY);
    if (string.str == NULL)
        exit(-1); //TODO vnitřní err
    bool inParentheses = false; //NL v závorkách se chová jako obyčejný whitespace
    int c;
    while (!foundToken) 
    {
        if(toDedent){
            token.type = TKN_DEDENTATION;
            toDedent--;
            foundToken = true;
            break;
        }
        c = getc(stream);
#ifdef DEBUG
        printf("char: '%c',\t state: %d,\t %s\n", c, state, string.str);
#endif
        EOF_AND_OUT_OF_ASCII_CHECK
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
            else if (c == '-') {foundToken = true; token.type = TKN_MINUS;}
            else if (c == '+') {foundToken = true; token.type = TKN_PLUS;}
            else if (c == '/') state = ST_DIVISION_SLASH;
            else if (c == '*') {foundToken = true; token.type = TKN_TIMES;}
            else if (c == ',') {foundToken = true; token.type = TKN_COMMA;}
            else if (c == ':') {foundToken = true; token.type = TKN_COLON;}
            else if (c == ';') {foundToken = true; token.type = TKN_SEMICOLON;}
            else if (c == '!') state = ST_NOT;
            else if (c == '=') state = ST_EQUAL_SIGN;
            else if (c == '>') state = ST_GREATER;
            else if (c == '<') state = ST_LESS;
            else if (('A' <= c && c <= 'Z')||('a' <= c && c <= 'z')) state = ST_ID;
            else if (c == '_') state = ST_EXT_UNDERSCORE;
            else if (c == ')') {foundToken = true; token.type = TKN_CLOSING_PARENTHESES; inParentheses = false;}
            else if (c == '(') {foundToken = true; token.type = TKN_OPENING_PARENTHESES; inParentheses = true;}
            else if (c == ']') {foundToken = true; token.type = TKN_CLOSING_SQUARE_PARENTHESES;}
            else if (c == '[') {foundToken = true; token.type = TKN_OPENING_SQUARE_PARENTHESES;}
            else if (c == '|') {foundToken = true; token.type = TKN_EXT_VERTICAL_LINE;}
            else if (c == '.') {foundToken = true; token.type = TKN_DOT;}
            else if (c == '"') {
                state = ST_OPENING_QUOTATION_MARK;
                string.length = 0;
                string.str[0] = '\0';
            }
            else if (c == '@'){
                printf("add\n");
                if (getc(stream) == 'm' &&
                    getc(stream) == 'a' &&
                    getc(stream) == 'i' &&
                    getc(stream) == 'n'){
                        token.type = TKN_AT_MAIN;
                        state = ST_START;
                        foundToken = true;
                    } else {free(string.str); exit(1);} //LEX ERR 1 
                } else if (c == 10) {
                if(!inParentheses){
                    token.type = TKN_NEWLINE;
                    foundToken = true;
                }
                state = ST_INDENTATION;
            } //ignoring the rest of ASCII symbols
            break;
        case ST_INDENTATION:
            unsigned spaces = 0;
            if(c == 9) { //tabulátor na začátku neprázdného řádku je lexikální chyba 1
                while ((0 <= (c = getc(stream)) && c <= 32) || c == 127) {;} // ignoring whitespaces
                EOF_AND_OUT_OF_ASCII_CHECK
                if (c != 10) {free(string.str); exit(1);} //LEX ERR 1
                token.type = TKN_NEWLINE; //TODO spíš ignor?
                foundToken = true;
                break;
            }
            while (c == ' '){
                spaces++; 
                c = getc(stream);
            }
            EOF_AND_OUT_OF_ASCII_CHECK
            if (c == 10){
                token.type = TKN_NEWLINE;
                foundToken = true;
                state = ST_INDENTATION;
                break;
            }
            ungetc(c, stream);
            state = ST_START;

            if (spaces != indentationStack.stack[indentationStack.top]){
                if (spaces > indentationStack.stack[indentationStack.top]){
                    push(&indentationStack, spaces);
                    token.type = TKN_INDENTATION;
                    foundToken = true;
                } else {
                    while (indentationStack.stack[indentationStack.top] > spaces){
                        indentationStack.top--;
                        toDedent++;
                    }
                    if (indentationStack.stack[indentationStack.top] != spaces)
                        exit(1);
                }
            }
            state = ST_START;
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
                ungetc(c, stream);
                token.type = TKN_INT;
                token.intValue = 0;
                state = ST_START;
                foundToken = true;
            }
            break;
        case ST_ZB_UNDERSCORE:
            if ('0' <= c && c <= '9') state = ST_ZERO_BEGINNING;
            else {free(string.str); exit(1);} //LEX ERR 1 
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
                ungetc(c, stream);
                state = ST_START;
                foundToken = true;
            }
            break;
        case ST_NZB_UNDERSCORE:
            if ('0' <= c && c <= '9') state = ST_NONZERO_BEGINNING;
            else {free(string.str); exit(1);} //LEX ERR 1 
            break;
        case ST_DOUBLE_DECIMAL_DOT:
            if ('0' <= c && c <= '9') {
                state = ST_DECIMAL_PART;
                stringAppend(&string, c);
            }
            else {free(string.str); exit(1);} //LEX ERR 1 
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
                token.doubleValue = strtod(string.str, NULL);
                ungetc(c, stream);
                state = ST_START;
                foundToken = true;
            }
            break;
        case ST_DP_UNDERSCORE:
            if ('0' <= c && c <= '9') state = ST_DECIMAL_PART;
            else {free(string.str); exit(1);} //LEX ERR 1 
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
            else {free(string.str); exit(1);} //LEX ERR 1 
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
            else {free(string.str); exit(1);} //LEX ERR 1 
            break;
        case ST_EXP_SING:
            if ('1' <= c && c <= '9') {
                state = ST_EXPONENT;
                stringAppend(&string, c);
            }
            else {free(string.str); exit(1);} //LEX ERR 1 
            break;
        case ST_EXPONENT:
            if ('0' <= c && c <= '9') {
                stringAppend(&string, c);
                break; //stays
            }
            else if (c == '_') state = ST_EXP_UNDERSCORE;
            else if (c == 'd' || c == 'D') state = ST_DOUBLE_SUFIX;
            else {
                token.type = TKN_DOUBLE;
                token.doubleValue = strtod(string.str, NULL);
                ungetc(c, stream);
                state = ST_START;
                foundToken = true;
            }
            break;
        case ST_EXP_UNDERSCORE:
            if ('0' <= c && c <= '9') {
                state = ST_EXPONENT;
                stringAppend(&string, c);
            }
            else {free(string.str); exit(1);} //LEX ERR 1 
            break;
        case ST_DOUBLE_SUFIX: //TODO remove sufixes
            token.type = TKN_DOUBLE;
            token.doubleValue = strtod(string.str, NULL);
            ungetc(c, stream);
            state = ST_START;
            foundToken = true;
            break;
        case ST_DIVISION_SLASH:
            if (c == '*') state = ST_OPENING_BLOCK_COMMENT;
            else if (c == '/') {
                state = ST_INDENTATION;
                while ((c=getc(stream)) != 10 && c != EOF) {;}
                foundToken = true;
                if(c == 10){
                    token.type = TKN_NEWLINE;
                    state = ST_INDENTATION;
                } else if (c == EOF){
                    token.type = TKN_EOF;
                    state = ST_START;
                }
                break;
            }
            else {
                token.type = TKN_DIVISION_SLASH;
                ungetc(c, stream);
                state = ST_START;
                foundToken = true;
            }
            break;
        case ST_LESS:
            if (c == '=') {foundToken = true; state = ST_START; token.type = TKN_LESS_EQUAL;}
            else if (c == '-') {foundToken = true; state = ST_START; token.type = TKN_EXT_LESS_MINUS;}
            else {
                token.type = TKN_LESS;
                ungetc(c, stream);
                state = ST_START;
                foundToken = true;
            }
            break;
        case ST_GREATER:
            if (c == '=') {foundToken = true; state = ST_START; token.type = TKN_GREATER_EQUAL;}
            else {
                token.type = TKN_GREATER;
                foundToken = true;
                ungetc(c, stream);
                state = ST_START;
            }
            break;
        case ST_EQUAL_SIGN:
            if (c == '=') {foundToken = true; state = ST_START; token.type = TKN_EQUAL;}
            else if (c == '>') {foundToken = true; state = ST_START; token.type = TKN_EXT_EQUAL_GREATER;}
            else {
                ungetc(c, stream);
                foundToken = true;
                token.type = TKN_ASSING;
                state = ST_START;
            } 
            break;
        case ST_NOT:
            if (c == '=') {foundToken = true; state = ST_START; token.type = TKN_NOT_EQUAL;} //TODO add state = ststart
            else {free(string.str); exit(1);} //LEX ERR 1 
            break;
        case ST_OPENING_BLOCK_COMMENT: //řeší i jiné stavy z FSM
            unsigned depthCount = 1;
            bool asNewLine = false;
            while (depthCount) {
                c = getc(stream);
                if (c == EOF || c < 0 || 127 < c) {free(string.str); exit(1);}
                else if (c == 10) asNewLine = true;
                else if (c == '/') {
                slash:
                    if ((c = getc(stream)) == '*')
                        depthCount++;
                    else if (c == 10) asNewLine = true; 
                    else if (c == '/') goto slash;
                    else if (c == EOF || c < 0 || 127 < c) {free(string.str); exit(1);}
                }
                else if (c == '*'){
                asterisk:
                    if ((c = getc(stream)) == '/')
                        depthCount--;
                    else if (c == 10) asNewLine = true;
                    else if (c == '*') goto asterisk;
                    else if ((depthCount && c == EOF) || c < 0 || 127 < c) {free(string.str); exit(1);}
                }
            }
            if (c == EOF) {foundToken = true; token.type = TKN_EOF; break;}
            if(asNewLine){
                while (true){
                    c = getc(stream);
                    EOF_AND_OUT_OF_ASCII_CHECK
                    else if (c == 10) {
                        token.type = TKN_NEWLINE;
                        foundToken = true;
                        state = ST_INDENTATION;
                    } else if (32 < c && c < 127) {free(string.str); exit(1);} //LEX ERR 1 
                }
            } 
            break;        
        case ST_EXT_UNDERSCORE:
            if (c == '_') {
                state = ST_ID_UNDERSCORES;
                stringAppend(&string, c);
            }
            else {
                token.type = TKN_EXT_UNDERSCORE;
                foundToken = true;
                state = ST_START;
                ungetc(c, stream);
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
            if (('0' <= c && c <= '9')||('A' <= c && c <= 'Z')||('a' <= c && c <= 'z')) {
                stringAppend(&string, c);
                break; //stays
            } else {
                token.type = TKN_ID;
                token.string = string.str;
                foundToken = true;
                state = ST_START;
                if (c == '.') {
                    if( string.length == 5 &&
                        string.str[0] == 'i' &&
                        string.str[1] == 'f' &&
                        string.str[2] == 'j' &&
                        string.str[3] == '2' &&
                        string.str[4] == '6' &&
                        (c = getc(stream)) == '*'){ 
                            token.type = TKN_IFJ26dot_asterisk;
                            foundToken = true;
                            state = ST_START;
                            break;
                    }
                }
                ungetc(c, stream);
                switch (string.length) //TODO SHORTEN, makro pro found state type a funkce pro cmp
                {
                    case 2: //if, do
                        COMPARE_WITH_KEYWORD(if, TKN_IF)
                        else COMPARE_WITH_KEYWORD(do, TKN_DO)
                        break;
                    case 3: //val, var, for, def, Int
                        if (STR_CMP(0, 'v') && STR_CMP(1, 'a')) {
                            if (STR_CMP(2, 'r')) TOKEN_FOUND_RETURN_TO_START(TKN_VAR)
                            else if (STR_CMP(2, 'l')) TOKEN_FOUND_RETURN_TO_START(TKN_VAL) 
                        }
                        else COMPARE_WITH_KEYWORD(for, TKN_EXT_FOR)
                        else COMPARE_WITH_KEYWORD(def, TKN_DEF)
                        else COMPARE_WITH_KEYWORD(Int, TKN_INT_KEYWORD)
                        break;
                    case 4: //else, then, Unit, type, case, List
                        COMPARE_WITH_KEYWORD(else, TKN_ELSE)
                        else COMPARE_WITH_KEYWORD(then, TKN_THEN)
                        else COMPARE_WITH_KEYWORD(Unit, TKN_UNIT_KEYWORD)
                        else COMPARE_WITH_KEYWORD(type, TKN_EXT_TYPE)
                        else COMPARE_WITH_KEYWORD(case, TKN_EXT_CASE)
                        else COMPARE_WITH_KEYWORD(List, TKN_EXT_LIST)
                        break;
                    case 5: //while, class, match
                        COMPARE_WITH_KEYWORD(while, TKN_WHILE)
                        else COMPARE_WITH_KEYWORD(class, TKN_EXT_CLASS)
                        else COMPARE_WITH_KEYWORD(match, TKN_EXT_MATCH)
                        break;
                    case 6: //return, Double, import, String
                        COMPARE_WITH_KEYWORD(return, TKN_RETURN)
                        else COMPARE_WITH_KEYWORD(Double, TKN_DOUBLE_KEYWORD)
                        else COMPARE_WITH_KEYWORD(import, TKN_IMPORT)
                        else COMPARE_WITH_KEYWORD(String, TKN_STRING_KEYWORD)
                        break;
                    case 7: //Boolean
                        COMPARE_WITH_KEYWORD(Boolean, TKN_BOOLEAN)
                        break;
                    case 9: //extension
                        COMPARE_WITH_KEYWORD(extension, TKN_EXT_EXTENSION)
                        break;
                }
            }
            break;
        case ST_OPENING_QUOTATION_MARK:
            if(c == '"') state = ST_TWO_QUOTATION_MARKS;
            else if (c == '\\') {
                state = ST_IN_ESC_SEQ;
                string.str[0] = '\\';
                string.str[1] = '\0';
                string.length = 1;
            }
            else if (c == 10) {free(string.str); exit(1);} //LEX ERR 1 
            else {
                state = ST_IN_STRING;
                string.str[0] = c;
                string.str[1] = '\0';
                string.length = 1;
            }
            break;
        case ST_TWO_QUOTATION_MARKS:
            if (c == '"') state = ST_MULTILINE_STRING;
            else { //empty string
                token.type = TKN_STRING;
                string.str[0] = '\0';
                string.length = 0;
                token.string = string.str;
                foundToken = true;
                state = ST_START;
                ungetc(c, stream);
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
                string.length = string.length - 3;
                string.str[string.length] = '\0';
                token.type = TKN_MULTILINE_STRING;
                token.string = string.str;
                foundToken = true;
                state = ST_START; //TODO check
            }
            else {
                state = ST_MULTILINE_STRING;
                stringAppend(&string, c);
            }
            break;
        case ST_IN_STRING:
            if (c == '"') {
                foundToken = true; 
                state = ST_START; 
                token.type = TKN_STRING;
                token.string = string.str;
            }
            else if (c == '\\') {
                state = ST_IN_ESC_SEQ;
                stringAppend(&string, c);
            }
            else if (c == 10) {free(string.str); exit(1);} //LEX ERR 1 
            else stringAppend(&string, c);
            break; //else just stays
        case ST_IN_ESC_SEQ:
            if (c == '"' || c == '\\' || c == 'n' || c == 'r' || c == 't') state = ST_IN_STRING;
            else {free(string.str); exit(1);} //LEX ERR 1 
            break;
        }
    }
    if(token.type != TKN_ID && token.type != TKN_STRING && token.type != TKN_MULTILINE_STRING)
        free(string.str);
#ifdef DEBUG
        printf("token found: %d\n", token.type);
#endif
    return token;
}