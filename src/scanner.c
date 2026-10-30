// scanner.c
// Autor: Filip Maxa, FIT

#include "scanner_private.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>

#define STR_CMP(i, c) (string.str[i] == (c))

token_t getToken(FILE* stream) {
    bool foundToken = false;
    static enum EState state = ST_START;
    static unsigned lastIndentation = 0;
    token_t token = {.type = TKN_NONE};
    string_t string = {.capacity = STRING_STARTING_CAPACITY, .length = 0, .str = NULL};
    string.str = (char*)malloc(STRING_STARTING_CAPACITY);
    if (string.str == NULL);
        abort(); //TODO vnitřní err
    bool inParentheses = false; //NL v závorkách se chová jako obyčejný whitespace
    int c;
    while (!foundToken) 
    {
        c = getc(stream);
        if(c == EOF)
            state = ST_EOF;
        if (c < 0 || 127 < c) {free(string.str); exit(1);} //LEX ERR 1 //MIMO ASCII
        switch (state) //https://share.gemini.google/zIizBiReNRVX
        {
        case ST_EOF:
            token.type = TKN_EOF;
            foundToken = true;
            break;
        case ST_START:
            string.str[0] = c;
            string.str[1] = '\0';
            string.length = 1;
            if(c == '0') state = ST_ZERO_BEGINNING;
            else if ('1' <= c && c <= '9') {
                token.intValue = (c - '0');
                state = ST_NONZERO_BEGINNING;
            }
            else if (c == '-') {foundToken = true; state = ST_START; token.type = TKN_MINUS;}
            else if (c == '+') {foundToken = true; state = ST_START; token.type = TKN_PLUS;}
            else if (c == '/') state = ST_DIVISION_SLASH;
            else if (c == '*') {foundToken = true; state = ST_START; token.type = TKN_TIMES;}
            else if (c == ',') {foundToken = true; state = ST_START; token.type = TKN_COMMA;}
            else if (c == ':') {foundToken = true; state = ST_START; token.type = TKN_COLON;}
            else if (c == ';') {foundToken = true; state = ST_START; token.type = TKN_SEMICOLON;}
            else if (c == '!') state = ST_NOT;
            else if (c == '=') state = ST_EQUAL_SIGN;
            else if (c == '>') state = ST_GREATER;
            else if (c == '<') state = ST_LESS;
            else if (('A' <= c && c <= 'Z')||('a' <= c && c <= 'z')) state = ST_ID;
            else if (c == '_') state = ST_EXT_UNDERSCORE;
            else if (c == ')') {foundToken = true; state = ST_START; token.type = TKN_CLOSING_PARENTHESES;}
            else if (c == '(') state = ST_OPENING_PARENTHESES;
            else if (c == '|') {foundToken = true; state = ST_START; token.type = TKN_EXT_VERTICAL_LINE;}
            else if (c == '"') {
                state = ST_OPENING_QUOTATION_MARK;
                string.length = 0;
                string.str[0] = '\0';
            }
            else if (c == '@') state = ST_AT_MAIN;
            else if (c == '10') {
                if(!inParentheses){
                    token.type = TKN_NEWLINE;
                    foundToken = true;
                }
                state = ST_INDENTATION;
            }
            else if (c <= 127) break; //ignoring remaining ASCII symbols
            break;
        case ST_AT_MAIN:
            if (getc(stream) == 'm' &&
                getc(stream) == 'a' &&
                getc(stream) == 'i' &&
                getc(stream) == 'n'){
                    token.type = TKN_AT_MAIN;
                    state = ST_START;
                    foundToken = true;
                }
            else {free(string.str); exit(1);} //LEX ERR 1 
            break;
        case ST_INDENTATION: //TODO add dedentation
            unsigned spaces = 0;
            if(c == 9) { //tabulátor na začátku neprázdného řádku je lexikální chyba 1
                while (0 <= (c = getc(stream)) && c <= 32 || c == 127) {;} // ignoring whitespaces
                if (c == EOF) {foundToken = true; token.type = TKN_EOF; break;}
                if (c != 10) {free(string.str); exit(1);} //LEX ERR 1
                token.type = TKN_NEWLINE;
                state = ST_INDENTATION; //redundant
                foundToken = true;
            }
            if (c < 0 || 127 < c) {free(string.str); exit(1);} //LEX ERR 1
            while ((c = getc(stream)) == ' ') spaces++;
            if (c < 0 || 127 < c) {free(string.str); exit(1);} //LEX ERR 1
            ungetc(c, stream);
            if (spaces != lastIndentation){
                token.type = (spaces > lastIndentation ? TKN_INDENTATION : TKN_DEDENTATION);
                foundToken = true;
                state = ST_START;
            }
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
                //token.doubleValue = string TODO
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
            else if (c == 'd', c == 'D') state = ST_DOUBLE_SUFIX;
            else {
                token.type = TKN_DOUBLE;
                //token.doubleValue = string TODO
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
        case ST_DOUBLE_SUFIX:
            token.type = TKN_DOUBLE;
            //token.doubleValue = string TODO
            ungetc(c, stream);
            state = ST_START;
            foundToken = true;
        case ST_DIVISION_SLASH:
            if (c == '*') state = ST_OPENING_BLOCK_COMMENT;
            else if (c == '/') state = ST_DOUBLE_SLASH_COMMENT;
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
            else {free(string.str); exit(1);} //LEX ERR 1 
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
                if (c == '10') asNewLine = true;
                else if (c == '/')
                    if ((c = getc(stream)) == '*')
                        depthCount++;
                    else if (c == '10') asNewLine = true; 
                else if (c == '*')
                    if ((c = getc(stream)) == '/')
                        depthCount--;
                    else if (c == '10') asNewLine = true;
                else if (c < 0 || 127 < c) {free(string.str); exit(1);} //LEX ERR 1 //MIMO ASCII
            }
            if(asNewLine){
                while ((c = getc(stream)) != 10 /*nl*/){
                    if(c == EOF){
                        state = ST_EOF;
                        break;
                    }
                    if(c < 0 || 32 < c && c != 127) {free(string.str); exit(1);} //LEX ERR 1 
                }
                token.type = TKN_NEWLINE;
                foundToken = true;
                state = ST_INDENTATION;
            } 
            break;        
        case ST_DOUBLE_SLASH_COMMENT:
            if (c == '10') state = ST_INDENTATION; //komentář jako LF 
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
            if (('0' <= c && c <= '9')||('A' <= c && 'Z' <= c)||('a' <= c && c <= 'z')) {
                stringAppend(&string, c);
                break; //stays
            } else {
                token.type = TKN_ID;
                token.string = string.str;
                foundToken = true;
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
                    } else {free(string.str); exit(1);} //LEX ERR 1  //TODO extensions?
                }
                switch (string.length)
                {
                    case 2: //if, do
                        if (STR_CMP(0, 'i') && STR_CMP(1, 'f')) {foundToken = true; state = ST_START; token.type = TKN_IF;}
                        else if (STR_CMP(0, 'd') && STR_CMP(1, 'o')) {foundToken = true; state = ST_START; token.type = TKN_DO;}
                        break;
                    case 3: //val, var, for, def, Int
                        if (STR_CMP(0, 'a') && STR_CMP(1, 'a')) {
                            if (STR_CMP(2, 'r')) {foundToken = true; state = ST_START; token.type = TKN_VAR;}
                            else if (STR_CMP(2, 'l')) {foundToken = true; state = ST_START; token.type = TKN_VAL;} }
                        else if (STR_CMP(0, 'f') && STR_CMP(1, 'o') && STR_CMP(2, 'r')) {foundToken = true; state = ST_START; token.type = TKN_EXT_FOR;}
                        else if (STR_CMP(0, 'd') && STR_CMP(1, 'e') && STR_CMP(2, 'f')) {foundToken = true; state = ST_START; token.type = TKN_DEF;}
                        else if (STR_CMP(0, 'I') && STR_CMP(1, 'n') && STR_CMP(2, 't')) {foundToken = true; state = ST_START; token.type = TKN_INT;}
                        break;
                    case 4: //else, then, Unit, type, case, List
                        if (STR_CMP(0, 'e') && STR_CMP(1, 'l') && STR_CMP(2, 's') && STR_CMP(3, 'e')) {foundToken = true; state = ST_START; token.type = TKN_ELSE;}
                        else if (STR_CMP(0, 't') && STR_CMP(1, 'h') && STR_CMP(2, 'e') && STR_CMP(3, 'n')) {foundToken = true; state = ST_START; token.type = TKN_THEN;}
                        else if (STR_CMP(0, 'U') && STR_CMP(1, 'n') && STR_CMP(2, 'i') && STR_CMP(3, 't')) {foundToken = true; state = ST_START; token.type = TKN_UNIT;}
                        else if (STR_CMP(0, 't') && STR_CMP(1, 'y') && STR_CMP(2, 'p') && STR_CMP(3, 'e')) {foundToken = true; state = ST_START; token.type = TKN_EXT_TYPE;}
                        else if (STR_CMP(0, 'c') && STR_CMP(1, 'a') && STR_CMP(2, 's') && STR_CMP(3, 'e')) {foundToken = true; state = ST_START; token.type = TKN_EXT_CASE;}
                        else if (STR_CMP(0, 'L') && STR_CMP(1, 'i') && STR_CMP(2, 's') && STR_CMP(3, 't')) {foundToken = true; state = ST_START; token.type = TKN_EXT_LIST;}
                        break;
                    case 5: //while, class, match
                        if (STR_CMP(0, 'w') && STR_CMP(1, 'h') && STR_CMP(2, 'i') && STR_CMP(3, 'l') && STR_CMP(4, 'e')) {foundToken = true; state = ST_START; token.type = TKN_WHILE;}
                        else if (STR_CMP(0, 'c') && STR_CMP(1, 'l') && STR_CMP(2, 'a') && STR_CMP(3, 's') && STR_CMP(4, 's')) {foundToken = true; state = ST_START; token.type = TKN_EXT_CLASS;}
                        else if (STR_CMP(0, 'm') && STR_CMP(1, 'a') && STR_CMP(2, 't') && STR_CMP(3, 'c') && STR_CMP(4, 'h')) {foundToken = true; state = ST_START; token.type = TKN_EXT_MATCH;}
                        break;
                    case 6: //return, Double, import, String
                        if (STR_CMP(0, 'r') && STR_CMP(1, 'e') && STR_CMP(2, 't') && STR_CMP(3, 'u') && STR_CMP(4, 'r') && STR_CMP(5, 'n')) {foundToken = true; state = ST_START; token.type = TKN_RETURN;}
                        else if (STR_CMP(0, 'D') && STR_CMP(1, 'o') && STR_CMP(2, 'u') && STR_CMP(3, 'b') && STR_CMP(4, 'l') && STR_CMP(5, 'e')) {foundToken = true; state = ST_START; token.type = TKN_DOUBLE;}
                        else if (STR_CMP(0, 'i') && STR_CMP(1, 'm') && STR_CMP(2, 'p') && STR_CMP(3, 'o') && STR_CMP(4, 'r') && STR_CMP(5, 't')) {foundToken = true; state = ST_START; token.type = TKN_IMPORT;}
                        else if (STR_CMP(0, 'S') && STR_CMP(1, 't') && STR_CMP(2, 'r') && STR_CMP(3, 'i') && STR_CMP(4, 'n') && STR_CMP(5, 'g')) {foundToken = true; state = ST_START; token.type = TKN_STRING;}
                        break;
                    case 9: //extension
                        if (STR_CMP(0, 'e') && STR_CMP(1, 'x') && STR_CMP(2, 't') && STR_CMP(3, 'e') && STR_CMP(4, 'n') && STR_CMP(5, 's') && STR_CMP(6, 'i') && STR_CMP(7, 'o') && STR_CMP(8, 'n')){
                            foundToken = true; state = ST_START; token.type = TKN_EXT_EXTENSION;
                        }
                        break;
                }
            }
            break;
        case ST_OPENING_PARENTHESES: //LF BEHAVES AS A WHITESPACE IN PARENTHESES
            inParentheses = true;
            if (c == ')') {
                inParentheses = false;
                token.type = TKN_UNIT;
                foundToken = true;
                state = ST_START;
            }
            else {
                token.type = TKN_OPENING_PARENTHESES;
                foundToken = true;
                state = ST_START;
                ungetc(c, stream);
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
                token.string = "\0";
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
            if (c == '"') {foundToken = true; state = ST_START; token.type = TKN_STRING;}
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
    if(token.type != TKN_ID && token.type != TKN_STRING && token.type != TKN_MULTILINE_STRING)
        free(string.str);
    return token;
}