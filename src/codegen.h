//codegen.h
// Autor: Martin Knotek, FIT
// Autor: Filip Maxa, FIT

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdbool.h>
#include <stdio.h>
#include "ast.h"

typedef enum{
  GF,
  TF,
  LF
} Frame_t;

/* Emits the IFJcode26 header and traverses the AST. */
bool codeGenerate(FILE *output, const AST_Node *root);

// Pregenerates while loop variables 
unsigned defvarPregen(AST_Node* node, unsigned scopecnt);

// Generates expressions
void expGenerate(AST_Node* node);

#endif