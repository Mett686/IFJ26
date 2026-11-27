// main.c
// Autor: Martin Knotek, FIT
// Autor: Filip Maxa, FIT


#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "ast.h"
#include "htab.h"
#include "abtDecoding.h"
#include "codegen.h"
#include "astPrint.h"


int main(int argc, char** argv) {
    AST_Node* ast;
    FILE* input = stdin;
    htab_t* htab;

    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "main: cannot open input file '%s'\n", argv[1]);
            return 1;
        }
    }

    htab = htabCreate(300);
    if (htab == NULL) {
        fprintf(stderr, "main: failed to allocate semantic symbol table\n");
        if (input != stdin) {
            fclose(input);
        }
        return 99;
    }

    ast = parser(input,htab);
    if (ast == NULL) {
        fprintf(stderr, "main: parser failed to create the syntax tree\n");
        htabDestroy(htab);
        if (input != stdin) {
            fclose(input);
        }
        return 99;
    }

    semanticCheck(ast, htab);
    if (!codeGenerate(stdout, ast)) {
        fprintf(stderr, "main: code generation failed\n");
        htabDestroy(htab);
        if (input != stdin) {
            fclose(input);
        }
        return 99;
    }
    // ast debug print
    ast_print(stderr, ast);

    if (input != stdin) {
        fclose(input);
    }
    htabDestroy(htab);
    return 0;
}