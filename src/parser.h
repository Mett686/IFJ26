
//autor: xmiklej00
#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "ast.h"

typedef enum{
PS_PLUS,
PS_STAR,
PS_SLASH,
PS_EQUAL,
PS_GREATER,
PS_LESS,
PS_LESSEQUAL,
PS_GREATEREQUAL,
PS_NOTEQUAL,
PS_MINUS,
PS_OPENING_PARENTHESES,
PS_CLOSING_PARENTHESES,
PS_END,
PS_ID,
PS_UNARYPLUS,
PS_UNARYMINUS,
PS_COUNT
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

typedef enum{
    STACK_TOKEN,
    STACK_TERM
}Symbol_Category;

typedef struct Stack{
    Symbol_Category category;
    Precedence_Symbol symbol;
    token_t token;
    struct Stack *next;
}Stack_t;



AST_Node* AST_inicial();




