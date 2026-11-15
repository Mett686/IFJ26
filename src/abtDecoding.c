#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "htab.h"
#include "../src/mock/mock_ast.h"  //fix later
#include "abtDecoding.h"

typedef struct htabData {
    int nested;
    char *string;
    bool isMutable;
    bool isFunction;
    int paramCount;
    int *paramTypes;
    int returnType;
} htab_data_t;

typedef struct htabItem {
    char* key;
    uint32_t hash;
    htab_data_t data;
} htabItem_t;

typedef struct htab_wrapper {
    struct htab_wrapper* parent;
    htab_t* htab;
} htab_wrapper_t;

static void blockCheck(const char* op, char* varName, tnode* parent, htab_t* globalHashTable, htab_wrapper_t* localHashTable) {
    if (localHashTable == NULL || globalHashTable == NULL) {
        exit(99);
    }

    if ((strcmp(op, "declare")) == 0) {
        htabItem_t* alreadyDefined = htabGet(localHashTable->htab, varName);
        if (alreadyDefined != NULL) {
            exit(4);
        }
        htab_data_t varData;
        varData.isFunction = false;
        varData.isMutable = parent->isVar;
        varData.paramCount = 0;
        varData.paramTypes = NULL;
        varData.returnType = parent->dataType;
        htabWrite(localHashTable->htab, varName, varData);

    } else if ((strcmp(op, "assign")) == 0) {
        htabItem_t* foundItem = NULL;
        htab_wrapper_t* searchScope = localHashTable;
        while (searchScope != NULL){
            foundItem = htabGet(searchScope->htab, varName);
            if (foundItem != NULL){
                break;
            }
            searchScope = searchScope->parent;     
        }
        //htabItem_t* globallyDefined = htabGet(globalHashTable, varName);
        if (foundItem == NULL) {
            exit(3);
        }

        if (foundItem->data.isMutable == false) {
            exit(7);
        }
    } else {
        exit(99);
    }
}

static void checkNode(tnode *parent, htab_t* hashTable, htab_wrapper_t** localHashTable, int* inBlock, int* currentReturnType) {
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
            tnode *opArg = parent->right;
            int index = 0;
            htabItem_t *funcDef = htabGet(hashTable, funcName);
            if (funcDef == NULL) {
                exit(3);
            }
            int isFunction = funcDef->data.isFunction;
            if (isFunction == 0){
                exit(3);
            }
            int parCount = funcDef->data.paramCount;
            int *paramTypes = funcDef->data.paramTypes;
            while (opArg != NULL){
                if (index >= parCount){
                    exit(5);
                } 

                int argType = opArg->left->dataType;
                opArg = opArg->right;
                int expectedType = paramTypes[index];
                if (argType != expectedType){
                    if (expectedType == TYPE_DOUBLE && argType == TYPE_INT){
                        //automaticka konverze z int na double
                    } else {
                        exit(6);
                    }
                }
                index++;
            }
            if (index != parCount){
                exit(5);
            }
            parent->dataType = funcDef->data.returnType;  
            break;
        }

        case opVarDeclare:
        {
            char *varName = parent->left->value;

            if (inBlock) {
                blockCheck("declare", varName, parent, hashTable, *localHashTable);
                break;
            }

            htabItem_t *varDef = htabGet(hashTable, varName);
            if (varDef != NULL) {
                exit(4);
            }

            if (leftType != rightType || parent->dataType != leftType) {
                if (leftType == TYPE_DOUBLE && rightType == TYPE_INT){
                    // automaticka konverze
                } 
                else {
                    exit(6);
                }
            }
            htab_data_t varData;
            varData.isFunction = false;
            varData.isMutable = parent->isVar;
            varData.paramCount = 0;
            varData.paramTypes = NULL;
            varData.returnType = parent->dataType;
            htabWriteNew(hashTable, varName, varData);
            break;
        }

        case funcDef:
        {
            *currentReturnType = parent->dataType;
            char *funcName = parent->left->value;
            htabItem_t *funcDef = htabGet(hashTable, funcName);

            if (funcDef != NULL){
                exit(4);
            }

            int count = 0;
            tnode *paramNode = parent->right; 
    
            while (paramNode != NULL) {
                count++;
                paramNode = paramNode->right;
            }

            int *typesArray = NULL;
            if (count > 0) {
                typesArray = malloc(count * sizeof(int));   
                paramNode = parent->right;
                int index = 0;
                while (paramNode != NULL) {
                    typesArray[index] = paramNode->left->dataType;
                    index++;
                    paramNode = paramNode->right;
                }
            }
            htab_data_t funcData;
            funcData.isFunction = true;
            funcData.paramCount = count;
            funcData.paramTypes = typesArray;
            funcData.returnType = parent->dataType;
            funcData.isMutable = false;
            htabWriteNew(hashTable, funcName, funcData);
            break;
        }

        case opAssign:
        {
            if (*inBlock) {
                char *varName = parent->left->value;
                blockCheck("assign", varName, parent, hashTable, *localHashTable);
            }

            if (leftType != rightType) {
                if (leftType == TYPE_DOUBLE && rightType == TYPE_INT){
                    // automaticka konverze
                } 
                else {
                    exit(6);
                }
            }

            char *varName = parent->left->value;

            htabItem_t *varDef = htabGet(hashTable, varName);
            if (varDef == NULL) {
                exit(3);
            }

            if (foundItem->data.isMutable == false) {
                exit(7);
            }
            break;
        }

        case opBlock:
        {
            *inBlock = 1;
            int dataSize = sizeof(htab_data_t);
            if (*localHashTable == NULL){ 
                *localHashTable = malloc(sizeof(htab_wrapper_t));  
                (*localHashTable)->htab = htabCreate(503, dataSize);
                (*localHashTable)->parent = NULL;
            } else {
                htab_wrapper_t* newScope = malloc(sizeof(htab_wrapper_t));
                newScope->htab = htabCreate(503, dataSize);
                newScope->parent = *localHashTable;
                *localHashTable = newScope;
            }          
            break;
        }

        case opEndBlock:
        {
            *inBlock = 0;
            if (*localHashTable != NULL) {
                *localHashTable = (*localHashTable)->parent;
            }
            
            if (*localHashTable == NULL) {
                *inBlock = 0;
            }
            break;
        }

        case opIf:
        case opWhile:
        {
            if (leftType != TYPE_BOOL){
                exit(6);
            }
            break;
        }
        case opReturn:
        {
            if (leftType != *currentReturnType){
                exit(6);
            }
            if (*currentReturnType == -1){
                exit(10);
            }
        }
    }
}

static void recursiveSemanticCheck(tnode *ptr, htab_t* hashTable, htab_wrapper_t** localHashTable, int* inBlock, int* currentReturnType){
    //postOrder browsing through the abstract tree
    if (ptr == NULL) {
        return;
    }
    recursiveSemanticCheck(ptr->left, hashTable, localHashTable, inBlock, currentReturnType);
    recursiveSemanticCheck(ptr->right, hashTable, localHashTable, inBlock, currentReturnType);
    if (ptr->left != NULL || ptr->right != NULL) {
        checkNode(ptr, hashTable, localHashTable, inBlock, currentReturnType);
    }
}

void semanticCheck(tnode *ptr, htab_t* hashTable) {
    int inBlock = 0;
    //htab_t* localHashTable = NULL;
    //htab_wrapper_t* localHashTable = malloc(sizeof(htab_wrapper_t));
    //localHashTable->parent = NULL;
    htab_wrapper_t* localHashTable = NULL;
    int currentReturnType = -1;
    recursiveSemanticCheck(ptr, hashTable, &localHashTable, &inBlock, &currentReturnType);
    htabItem_t *mainFunc = htabGet(hashTable, "@main");
    if (mainFunc == NULL) {
        exit(3);
    }

    if (mainFunc->value != NULL){
        exit(5);
    }
}