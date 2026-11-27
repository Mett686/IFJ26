#ifndef AST_PRINT_H
#define AST_PRINT_H

#include <stdio.h>
#include "ast.h"

void ast_print(FILE *out, const AST_Node *root);

#endif