// Implementace překladače imperativního jazyka IFJ26
// stack.c
// Implementace zásobníku pro syntaktickou analýzu

// Autor: Jakub Miklenčič (xmiklej00)

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "scanner.h"
#include "parser.h"

_Noreturn static void stack_exit(int code, const char* file, int line) {
    fprintf(stderr, "stack: allocation fatal error %d at %s:%d\n", code, file, line);
    exit(code);
}

#define exit(code) stack_exit(code, __FILE__, __LINE__)

Stack* stack_initialize() {
    Stack* new_stack = calloc(1, sizeof(Stack));
    if (new_stack != NULL) {
        new_stack->next = NULL;
        new_stack->category = STACK_TERM;
        new_stack->symbol = PS_END;

        return new_stack;
    } else {
        exit(99);
    }
}

Stack* stack_push_precedence(Stack* stack, Precedence precedence) {
    Stack* new_stack = calloc(1, sizeof(Stack));
    if (new_stack == NULL) {
        exit(99);
    }

    new_stack->precedence = precedence;
    new_stack->category = STACK_PRECEDENCE;

    if (stack->category == STACK_TERM) {
        new_stack->next = stack;
        return new_stack;
    }

    Stack* previous = stack;
    Stack* current = stack->next;
    while (current->category != STACK_TERM) {
        previous = current;
        current = current->next;
    }
    previous->next = new_stack;
    new_stack->next = current;
    return stack;
}

Stack* stack_push(Stack* stack, Symbol_Category category, Precedence_Symbol symbol, token_t token) {
    Stack* new_stack = calloc(1, sizeof(Stack));
    if (new_stack == NULL) {
        exit(99);

    } else {

        new_stack->next = stack;
        new_stack->category = category;
        new_stack->symbol = symbol;
        new_stack->token = token;
        return new_stack;
    }
}

Stack* stack_push_AST_node(Stack* stack, AST_Node* tree) {
    Stack* new_stack = calloc(1, sizeof(Stack));
    if (new_stack == NULL) {
        exit(99);

    } else {
        new_stack->next = stack;
        new_stack->category = STACK_NONTERM;
        new_stack->symbol = PS_EXP;
        new_stack->tree = tree;
        return new_stack;
    }
}

Stack* stack_pop(Stack* stack) {
    Stack* tmp = stack->next;
    free(stack);

    return tmp;
}

Precedence_Symbol stack_top_precedence_symbol(Stack* stack) {
    Stack* tmp = stack;
    while (tmp->category != STACK_TERM) {
        tmp = tmp->next;
    }
    return tmp->symbol;
}
