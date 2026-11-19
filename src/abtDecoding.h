#include "ast.h"

#define TYPE_INT 1
#define TYPE_FLOAT 2
#define TYPE_BOOL 3
#define TYPE_DOUBLE 4

void semanticCheck(AST_Node* ptr, htab_t* hashTable);

