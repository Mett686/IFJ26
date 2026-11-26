//autor: xmiklej00

#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "parser.h"

typedef enum{
    STACK_TERM,
    STACK_NONTERM,
    STACK_PRECEDENCE
}Symbol_Category;


typedef struct Stack{
    Symbol_Category category;
    Precedence_Symbol symbol;
    Precedence precedence;
    token_t token;
    AST_Node* tree;
    struct Stack *next;
}Stack;


Stack* stack_initialize();
Stack* stack_push_precedence(Stack *stack, Precedence precedence);
Stack* stack_push(Stack *stack,Symbol_Category category,Precedence_Symbol symbol, token_t token);
Stack* stack_pop(Stack *stack);
Precedence_Symbol stack_top_precedence_symbol(Stack *stack);
Stack* stack_push_AST_node(Stack *stack,AST_Node *tree);
Stack* parser_check_exp_rules(Stack *stack);
Precedence_Symbol parser_TKN_to_PS(token_t token,Stack*stack);
AST_Node* parser_exp_to_tree(Stack *stack,AST_Node *tree);
void parser_expression(FILE *stream, token_t *token, AST_Node *node,Stack* stack);
#endif