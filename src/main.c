// main.c
// Autor: Martin Knotek, FIT
// Autor: Filip Maxa, FIT


#include "parser.h"
#include "ast.h"
#include "htab.h"
#include "abtDecoding.h"
#include "codegen.h"


int main () {
    AST_Node* ast;
    htab_t* htab = htabCreate(300);

    ast = parser(stdin);
    semanticCheck(ast, htab);
    codeGenerate(stdout, ast);
    return 0;
}