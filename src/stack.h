//autor: xmiklej00

#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "parser.h"

typedef enum{
    STACK_TOKEN,
    STACK_TERM
}Symbol_Category;


typedef struct Stack{
    Symbol_Category category;
    Precedence_Symbol symbol;
    Precedence precedence;
    token_t token;
    struct Stack *next;
}Stack;


Stack* stack_initialize();
Stack* stack_push_precedence(Stack *stack, Precedence precedence);
Stack* stack_push(Stack *stack,Symbol_Category category,Precedence_Symbol symbol, token_t *token);
Stack* stack_pop(Stack *stack);

#endif