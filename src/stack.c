//autor: xmiklej00
#include <stdio.h>
#include "stack.h"
#include "scanner.h"
#include "parser.h"

Stack * stack_iniciaize(){
    Stack *new_stack=calloc(1,sizeof(Stack));
    if(new_stack!=NULL){
        new_stack->next=NULL;
        new_stack->category=STACK_TERM;
        new_stack->symbol=PS_END;
        
        return new_stack;
    }else{
        //ERROR
        return NULL;
    }
}

Stack* stack_push_precedence(Stack *stack,Precedens precedens){
    Stack *new_stack = calloc(1,sizeof(Stack));
    if(new_stack==NULL){
        //ERROR
        return NULL;
    }else{
        new_stack->next = stack;
        new_stack->precedenc = precedens;
        return new_stack;
    }
}



Stack* stack_push(Stack *stack,Symbol_Category category,Precedens_Symbol symbol, token_t *token){
    Stack *new_stack = malloc(sizeof(Stack));
    if(new_stack==NULL){
        //ERROR
        return NULL;
    }else{
        new_stack->next = stack;
        new_stack->category=category;
        new_stack->symbol=symbol;
        new_stack->token = *token;
        return new_stack;
    }
}

Stack* stack_pop(Stack *stack){
    Stack *tmp= stack->next;
    free(stack);
    
    return tmp;
}

