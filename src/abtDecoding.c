#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "htab.h"
#include "../src/mock/mock_ast.h"  //fix later
#include "abtDecoding.h"

static void blockCheck(const char* op, char* varName, char* varValue, htab_t* globalHashTable, htab_t* localHashTable) {
    if (localHashTable == NULL || globalHashTable == NULL) {
        exit(99);
    }

    if ((strcmp(op, "declare")) == 0) {
        htabItem_t* alreadyDefined = htabGet(localHashTable, varName);
        if (alreadyDefined != NULL) {
            exit(4);
        }

        htabWrite(localHashTable, varName, varValue);

    } else if ((strcmp(op, "assign")) == 0) {
        htabItem_t* locallyDefined = htabGet(localHashTable, varName);
        htabItem_t* globallyDefined = htabGet(globalHashTable, varName);
        if (locallyDefined == NULL && globallyDefined == NULL) {
            exit(3);
        }

    } else {
        exit(99);
    }
}

static void checkNode(tnode *parent, htab_t* hashTable, htab_t** localHashTable, int* inBlock) {
    int leftType;
    int rightType;

    if (parent->left != NULL) {
        leftType = parent->left->dataType;
    } else {
        leftType = -1;
    }

    if (parent->right != NULL) {
        rightType = parent->right->dataType;
    } else {
        rightType = -1;
    }

    switch (parent->dataType) {
        case opAdd:
        case opDec:
        case opMul:
        case opDiv:
            if (leftType == rightType) {
                parent->dataType = leftType;
            } else {
                exit(6);
            }
            break;

        case opMod:
            if (leftType == TYPE_INT && rightType == TYPE_INT) {
                parent->dataType = TYPE_INT;
            } else {
                exit(6);
            }
            break;

        case opEqual:
        case opLesser:
        case opBigger:
            if (leftType == rightType) {
                parent->dataType = TYPE_BOOL;
            } else {
                exit(6);
            }
            break;

        case opCall:
        {
            char *funcName = parent->left->value;
            htabItem_t *funcDef = htabGet(hashTable, funcName);
            if (funcDef == NULL) {
                exit(3);
            }
            break;
        }

        case opDeclare:
        {
            char *varName = parent->left->value;

            if (inBlock) {
                blockCheck("declare", varName, parent->right->value, hashTable, *localHashTable);
            }

            htabItem_t *varDef = htabGet(hashTable, varName);
            if (varDef != NULL) {
                exit(4);
            }

            if (leftType != rightType || parent->dataType != leftType) {
                exit(6);
            }

            htabWriteNew(hashTable, varName, parent->right);
            break;
        }

        case opAssign:
        {
            if (*inBlock) {
                char *varName = parent->left->value;
                blockCheck("assign", varName, parent->right->value, hashTable, *localHashTable);
            }
            if (leftType != rightType) {
                exit(6);
            }

            char *varName = parent->left->value;

            htabItem_t *varDef = htabGet(hashTable, varName);
            if (varDef == NULL) {
                exit(3);
            }
            break;
        }

        case opBlock:
        {
            *inBlock = 1;
            int dataSize = sizeof(leftType);
            *localHashTable = htabCreate(20, dataSize);
            break;
        }

        case opEndBlock:
        {
            *inBlock = 0;
            break;
        }
    }
}

static void recursiveSemanticCheck(tnode *ptr, htab_t* hashTable, htab_t** localHashTable, int* inBlock){
    //postOrder browsing through the abstract tree
    if (ptr == NULL) {
        return;
    }
    recursiveSemanticCheck(ptr->left, hashTable, localHashTable, inBlock);
    recursiveSemanticCheck(ptr->right, hashTable, localHashTable, inBlock);
    if (ptr->left != NULL || ptr->right != NULL) {
        checkNode(ptr, hashTable, localHashTable, inBlock);
    }
}

void semanticCheck(tnode *ptr, htab_t* hashTable) {
    int inBlock = 0;
    htab_t* localHashTable = NULL;
    recursiveSemanticCheck(ptr, hashTable, &localHashTable, &inBlock);
}