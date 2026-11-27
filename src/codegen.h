// Implementace překladače imperativního jazyka IFJ26
// codegen.h
// Rozhraní generátoru výstupního kódu

// Autor: Martin Knotek (xknotem00)
// Autor: Filip Maxa (xmaxafi00)

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdbool.h>
#include <stdio.h>
#include "ast.h"

typedef enum { GF, TF, LF } Frame_t;

/* Emits the IFJcode26 header and traverses the AST. */
bool codeGenerate(FILE* output, const AST_Node* root);

// Predeclares variables in while loop bodies.
void defvarPregen(AST_Node* node);

// Generates expressions
void expGenerate(const AST_Node* node);

#endif