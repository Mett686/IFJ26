//autor: xmiklej00
#include <stdio.h>
#include "scanner.h"
#include <stdbool.h>

typedef enum{
    STACK_TOKEN,
    STACK_TERM
}Symbol_Category;


typedef struct Stack{
    Symbol_Category category;
    Precedence_Symbol symbol;
    Precedence precedence;
    token_t token;
    Stack *next;
}Stack;


Stack* stack_initialize();
Stack* stack_push(Stack *stack,Symbol_Category category,Precedence_Symbol symbol, token_t *token);
Stack* stack_pop(Stack *stack);