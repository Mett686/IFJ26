// scanner_tester.c
// Autor: Filip Maxa, FIT

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"

void token_print(token_t* token, FILE* outputFile){
switch (token->type) //https://share.gemini.google/exHWaq43ZcTA
    {
    case TKN_OPENING_SQUARE_PARENTHESES:
        fprintf(outputFile, "['[', ] ");
        break;
    case TKN_CLOSING_SQUARE_PARENTHESES:
        fprintf(outputFile, "[']', ] ");
        break;
    case TKN_DOT:
        fprintf(outputFile, "['.',] ");
        break;
    case TKN_BOOLEAN:
        fprintf(outputFile, "[Boolean, ] ");
        break;
    case TKN_ASSING:
        fprintf(outputFile, "[=, ] ");
        break;
    case TKN_ID:
        fprintf(outputFile, "[ID, %s] ", token->string);
        break;
    case TKN_AT_MAIN:
        fprintf(outputFile, "[@main, ] ");
        break;
    case TKN_EOF:
        fprintf(outputFile, "[EOF, ] ");
        break;
    case TKN_INT:
        fprintf(outputFile, "[int_lit, %d] ", token->intValue);
        break;
    case TKN_INT_KEYWORD:
        fprintf(outputFile, "[Int, ] "); 
        break;
    case TKN_DOUBLE:
        fprintf(outputFile, "[double_lit, %lf] ", token->doubleValue);    
        break;
    case TKN_DOUBLE_KEYWORD:
        fprintf(outputFile, "[Double, ] ");
        break;
    case TKN_UNIT_KEYWORD:
        fprintf(outputFile, "[Unit, ] ");
        break;
    case TKN_STRING_KEYWORD:
        fprintf(outputFile, "[String, ] ");
        break;
    case TKN_STRING:
        fprintf(outputFile, "[string_lit, {{{%s}}}] ", token->string);
        break;
    case TKN_MULTILINE_STRING:
        fprintf(outputFile, "[multiline_string, {%s}] ", token->string);
        break;
    case TKN_VAL:
        fprintf(outputFile, "[val, ] ");
        break;
    case TKN_VAR:
        fprintf(outputFile, "[var, ] ");
        break;
    case TKN_DEF:
        fprintf(outputFile, "[def, ] ");
        break;
    case TKN_DO:
        fprintf(outputFile, "[do, ] ");
        break;
    case TKN_WHILE:
        fprintf(outputFile, "[while, ] ");
        break;
    case TKN_IF:
        fprintf(outputFile, "[if, ] ");
        break;
    case TKN_ELSE:
        fprintf(outputFile, "[else, ] ");
        break;
    case TKN_THEN:
        fprintf(outputFile, "[then, ] ");
        break;
    case TKN_IMPORT:
        fprintf(outputFile, "[import, ] ");
        break;
    case TKN_IFJ26dot_asterisk:
        fprintf(outputFile, "[ifj.*, ] ");
        break;
    case TKN_RETURN:
        fprintf(outputFile, "[return, ] ");
        break;
    case TKN_NEWLINE:
        fprintf(outputFile, "[NewLine, ]\n");
        break;
    case TKN_INDENTATION:
        fprintf(outputFile, "[INDENT, ] ");
        break;
    case TKN_DEDENTATION:
        fprintf(outputFile, "[DEDENT, ] ");
        break;
    case TKN_MINUS:
        fprintf(outputFile, "[-, ] ");
        break;
    case TKN_PLUS:
        fprintf(outputFile, "[+, ] ");
        break;
    case TKN_COMMA:
        fprintf(outputFile, "[',', ] ");
        break;
    case TKN_COLON:
        fprintf(outputFile, "[:, ] ");
        break;
    case TKN_SEMICOLON:
        fprintf(outputFile, "[;, ] ");
        break;
    case TKN_DIVISION_SLASH:
        fprintf(outputFile, "[/, ] ");
        break;
    case TKN_TIMES:
        fprintf(outputFile, "[*, ] ");
        break;
    case TKN_NOT_EQUAL:
        fprintf(outputFile, "[!=, ] ");
        break;
    case TKN_EQUAL:
        fprintf(outputFile, "[==, ] ");
        break;
    case TKN_LESS:
        fprintf(outputFile, "[<, ] ");
        break;
    case TKN_LESS_EQUAL:
        fprintf(outputFile, "[<=, ] ");
        break;
    case TKN_GREATER:
        fprintf(outputFile, "[>, ] ");
        break;
    case TKN_GREATER_EQUAL:
        fprintf(outputFile, "[>=, ] ");
        break;
    case TKN_OPENING_PARENTHESES:
        fprintf(outputFile, "[(, ] ");
        break;
    case TKN_CLOSING_PARENTHESES:
        fprintf(outputFile, "[), ] ");
        break;
    case TKN_EXT_CASE:
        fprintf(outputFile, "[case, ] ");
        break;
    case TKN_EXT_CLASS:
        fprintf(outputFile, "[class, ] ");
        break;
    case TKN_EXT_EXTENSION:
        fprintf(outputFile, "[extension, ] ");
        break;
    case TKN_EXT_FOR:
        fprintf(outputFile, "[for, ] ");
        break;
    case TKN_EXT_LIST:
        fprintf(outputFile, "[List, ] ");
        break;
    case TKN_EXT_MATCH:
        fprintf(outputFile, "[match, ] ");
        break;
    case TKN_EXT_TYPE:
        fprintf(outputFile, "[type, ] ");
        break;
    case TKN_EXT_EQUAL_GREATER:
        fprintf(outputFile, "[=>, ] ");
        break;
    case TKN_EXT_VERTICAL_LINE:
        fprintf(outputFile, "[|, ] ");
        break;
    case TKN_EXT_LESS_MINUS:
        fprintf(outputFile, "[<-, ] ");
        break;
    case TKN_EXT_UNDERSCORE:
        fprintf(outputFile, "[_, ] ");
        break;
    default:
        break;
    }
}

int main (int argc, char* argv[]){
    if (argc != 2)
        return -1;
    FILE* stream = fopen(argv[1], "r");
    if (stream == NULL)
        return -1;
    FILE* file = fopen("scanner_tester_output.txt", "w");
    if (file == NULL)
        exit(-123);
    token_t token = getToken(stream);
    while (token.type != TKN_EOF) {
        token_print(&token, file);
        if(token.type == TKN_ID || token.type == TKN_STRING || token.type == TKN_MULTILINE_STRING)
            free(token.string);
        token = getToken(stream);
    }
    token_print(&token, file);
    fclose(file);
    fclose(stream);
    return 0;
}