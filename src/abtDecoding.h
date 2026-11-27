// Implementace překladače imperativního jazyka IFJ26
// abtDecoding.h
// Rozhraní sémantické analýzy

// Autor: Marek Heckelmoser (xheckem00)

#include "ast.h"

void semanticCheck(AST_Node* ptr, htab_t* hashTable);