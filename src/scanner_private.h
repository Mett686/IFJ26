// scanner_private.h
// Autor: Filip Maxa, FIT

#include <stdlib.h>
#include <stdio.h>
#define STRING_STARTING_CAPACITY 64
#define STACK_STARTING_CAPACITY 10

typedef struct {
    unsigned capacity;
    unsigned length; //bez NTC
    char* str;
} string_t;

void stringAppend(string_t* string, int c){
    if(string->length == string->capacity + 1){
        string->str = realloc(string->str, string->capacity*2);
        if(string->str == NULL)
            exit(-1);
        string->capacity = string->capacity*2;
    }
    string->str[string->length++] = (char)c;
    string->str[string->length] = '\0';
}

typedef struct {
    unsigned top;
    unsigned capacity;
    unsigned* stack;
} indentation_stack_t;


indentation_stack_t* push(indentation_stack_t* stack, unsigned spaces){
    if(stack->top + 1 == stack->capacity){
        stack->stack = realloc(stack->stack, stack->capacity*2);
        if(stack->stack == NULL)
            exit(-1);
        stack->capacity *= 2;
    }
    stack->top++;
    stack->stack[stack->top] = spaces;
    return stack;
}
