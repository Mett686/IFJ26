
//autor: xmiklej00

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
PS_COUNT,
PS_EXP
}Precedence_Symbol;

typedef enum{
P_GREATER,
P_LESS,
P_EQUAL,
P_NONE
}Precedence;

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

AST_Node* AST_inicial(void);
AST_Node* create_node(Node_type type);
AST_Node* parser(FILE *stream,htab_t* table);
void parser_prolog(FILE* stream,token_t *token );
AST_Node* parser_definitionList(FILE *stream, token_t *token, AST_Node *tree,htab_t* table);
void parser_defFunction(FILE *stream, token_t *token, AST_Node *node,htab_t * table);
void parser_defMainFunc(FILE *stream, token_t *token, AST_Node *node);
void parser_type(FILE *stream, token_t *token, AST_Node *node);
void parser_body(FILE *stream, token_t *token, AST_Node *node);
void parser_block(FILE *stream, token_t *token, AST_Node *node);
void parser_statementList(FILE *stream, token_t *token, AST_Node *node);
void parser_between(FILE *stream, token_t *token);
void parser_valDef(FILE *stream, token_t *token, AST_Node *node);
void parser_optType(FILE *stream, token_t *token, AST_Node *node);
void parser_varDef(FILE *stream, token_t *token, AST_Node *node);
void parser_ifStatement(FILE *stream, token_t *token, AST_Node *node);
void parser_elsePart(FILE *stream, token_t *token, AST_Node *node);
void parser_whileLoop(FILE *stream, token_t *token, AST_Node *node);
void parser_returnRest(FILE *stream, token_t *token, AST_Node *node);
void parser_parameterList(FILE *stream, token_t *token, AST_Node *node);
void parser_parameter(FILE *stream, token_t *token, AST_Node *node);
void parser_parameterListRest(FILE *stream, token_t *token, AST_Node *node);
void parser_statementRest(FILE *stream, token_t *token, AST_Node *node);
void parser_argumentList(FILE *stream, token_t *token, AST_Node *node);
void parser_argumentListRest(FILE *stream, token_t *token, AST_Node *node);
Precedence parser_get_precedence(Precedence_Symbol stack_symbol, Precedence_Symbol coming_symbol);
void parser_expression(FILE *stream, token_t *token, AST_Node *node);
void parser_exp_to_tree(Stack *stack,AST_Node *tree);


#endif