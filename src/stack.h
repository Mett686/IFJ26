//autor: xmiklej00

#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "parser.h"
//
typedef enum{
    STACK_TERM,
    STACK_NONTERM,
    STACK_PRECEDENCE
}Symbol_Category;

//
typedef struct Stack{
    Symbol_Category category;
    Precedence_Symbol symbol;
    Precedence precedence;
    token_t token;
    AST_Node* tree;
    struct Stack *next;
}Stack;

//make stack
Stack* stack_initialize();
//pushes the precedence on stack use before stack_push
Stack* stack_push_precedence(Stack *stack, Precedence precedence);
//push term or nonterm on stack
Stack* stack_push(Stack *stack,Symbol_Category category,Precedence_Symbol symbol, token_t token);
//take the top out from stack
Stack* stack_pop(Stack *stack);
//give u back first term symbol
Precedence_Symbol stack_top_precedence_symbol(Stack *stack);
//pushes non term symbols
Stack* stack_push_AST_node(Stack *stack,AST_Node *tree);
//cheks rules between nonterms and if they are true then use and put it together
Stack* parser_check_exp_rules(Stack *stack);
//checks given token and top stack value and then decite which precedens symbol it is
Precedence_Symbol parser_TKN_to_PS(token_t token,Stack*stack);
//make sure on the stack is only one non terminal and push it out and put it in the AST tree 
AST_Node* parser_exp_to_tree(Stack *stack,AST_Node *tree);
//put tokens in stack and calls parser_check_exp_rules
void parser_expression(FILE *stream, token_t *token, AST_Node *node,Stack* stack);

#endif