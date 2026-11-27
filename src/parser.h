// Implementace překladače imperativního jazyka IFJ26
// parser.h
// Rozhraní syntaktického analyzátoru

// Autor: Jakub Miklenčič (xmiklej00)

#ifndef PARSER_H
#define PARSER_H


#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "ast.h"
#include "htab.h"


typedef enum{
PS_PLUS,
PS_TIMES,
PS_DIVISION,
PS_EQUAL,
PS_GREATER,
PS_LESS,
PS_LESS_EQUAL,
PS_GREATER_EQUAL,
PS_NOT_EQUAL,
PS_MINUS,
PS_OPENING_PARENTHESES,
PS_CLOSING_PARENTHESES,
PS_END,
PS_ID,
PS_UNARY_PLUS,
PS_UNARY_MINUS,
PS_EXP,
PS_COUNT
}Precedence_Symbol;

typedef enum{
P_GREATER,
P_LESS,
P_EQUAL,
P_NONE
}Precedence;
//table for precedence
static const char table [PS_COUNT][PS_COUNT] = {
    {'>','<','<','>','>','>','>','>','>','>','<','>','>','<','<','<'},
    {'>','>','>','>','>','>','>','>','>','>','<','>','>','<','<','<'},
    {'>','>','>','>','>','>','>','>','>','>','<','>','>','<','<','<'},
    {'<','<','<',' ','<','<','<','<',' ','<','<','>','>','<','<','<'},
    {'<','<','<','>',' ',' ',' ',' ','>','<','<','>','>','<','<','<'},
    {'<','<','<','>',' ',' ',' ',' ','>','<','<','>','>','<','<','<'},
    {'<','<','<','>',' ',' ',' ',' ','>','<','<','>','>','<','<','<'},
    {'<','<','<','>',' ',' ',' ',' ','>','<','<','>','>','<','<','<'},
    {'<','<','<',' ','<','<','<','<',' ','<','<','>','>','<','<','<'},
    {'>','<','<','>','>','>','>','>','>','>','<','>','>','<','<','<'},
    {'<','<','<','<','<','<','<','<','<','<','<','=',' ','<','<','<'},
    {'>','>','>','>','>','>','>','>','>','>',' ','>','>',' ',' ',' '},
    {'<','<','<','<','<','<','<','<','<','<','<','>',' ','<','<','<'},
    {'>','>','>','>','>','>','>','>','>','>','<','>','>',' ',' ',' '},
    {'>','>','>','>','>','>','>','>','>','>','<','>','>','<','>','>'},
    {'>','>','>','>','>','>','>','>','>','>','<','>','>','<','<','<'},
};
//make the start for tree
AST_Node* AST_inicial(void);
//create nodes with type for tree
AST_Node* create_node(Node_type type);
// the starter function for parser
AST_Node* parser(FILE *stream,htab_t* table);
//make sure there is IMPOR if26.*
void parser_prolog(FILE* stream,token_t *token );
//deside if it is def function or def main function
AST_Node* parser_definitionList(FILE *stream, token_t *token, AST_Node *tree,htab_t* table);
//make sure the definition is corect and call body and parameter list
void parser_defFunction(FILE *stream, token_t *token, AST_Node *node,htab_t * table);
//make sure the main definition is corect and call body and parameter list
void parser_defMainFunc(FILE *stream, token_t *token, AST_Node *node);
// fill the end node with type of id
void parser_type(FILE *stream, token_t *token, AST_Node *node);
//decide if there is "\n" or id and decide if there is one line body or BLock
void parser_body(FILE *stream, token_t *token, AST_Node *node);
//make sure there is INDENT and DEDENT
void parser_block(FILE *stream, token_t *token, AST_Node *node);
//decite on whitch statemend should go next or  if ID call statementRest
void parser_statementList(FILE *stream, token_t *token, AST_Node *node);
//after some statements make shure there is deviding character
void parser_between(FILE *stream, token_t *token);
//control if val definition is corect and call expretion or statment rest after =
void parser_valDef(FILE *stream, token_t *token, AST_Node *node);
// called if the type is only otional and kontrol if the type is corectli defined
void parser_optType(FILE *stream, token_t *token, AST_Node *node);
//control if val definition is corect and call expretion or statment rest after =
void parser_varDef(FILE *stream, token_t *token, AST_Node *node);
//make sure there is if, than and then call body
void parser_ifStatement(FILE *stream, token_t *token, AST_Node *node);
//make sure there is else part
void parser_elsePart(FILE *stream, token_t *token, AST_Node *node);
//make sure there is while then call expretion do and then block
void parser_whileLoop(FILE *stream, token_t *token, AST_Node *node);
//when comes return to statment list it takes return and all after that comes here
void parser_returnRest(FILE *stream, token_t *token, AST_Node *node);
//in def function after id and openinng parenthes comes paramet list make sure between parameters is  comma
void parser_parameterList(FILE *stream, token_t *token, AST_Node *node);
//its one part of paramet list
void parser_parameter(FILE *stream, token_t *token, AST_Node *node);
//after first parameter all of that comes here and it cicles here
void parser_parameterListRest(FILE *stream, token_t *token, AST_Node *node);
//if statement starts with id u call this function and this will decite if its function call or expretion
void parser_statementRest(FILE *stream, token_t *token, AST_Node *node);
// when function call happens then after id and opening parentles it comes here and make expretions for every argument
void parser_argumentList(FILE *stream, token_t *token, AST_Node *node);
//after first argument it comes here and cicles here
void parser_argumentListRest(FILE *stream, token_t *token, AST_Node *node);
//put here two precedence symbols and it will give u precedence between them
Precedence parser_get_precedence(Precedence_Symbol stack_symbol, Precedence_Symbol coming_symbol);
//before expretion this comes and inicialized stack and push forst token here then call parser expretion
void parser_expression_inicial(FILE *stream, token_t *token, AST_Node *node);
//if u need to take id and then look for next to decide what u want to do u use this 
//function where it push id on stack and stack and token push to next function
void parser_exp_prepare(FILE* stream, token_t *token , token_t id,AST_Node *node);


#endif