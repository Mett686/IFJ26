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
    Precedens_Symbol symbol;
    Precedens recedece;
    token_t token;
    Stack *next;
}Stack;


Stack* stack_iniciaize();
Stack* stack_push(Stack *stack,Symbol_Category category,Precedens_Symbol symbol, token_t *token);
Stack* stack_pop(Stack *stack);