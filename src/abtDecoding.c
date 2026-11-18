#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "htab.h"
#include "../src/mock/mock_ast.h"  //fix later
#include "abtDecoding.h"


static void blockCheck(const char* op, char* varName, tnode* parent, htab_t* globalHashTable, htab_t* localHashTable) {
    if (localHashTable == NULL || globalHashTable == NULL) {
        fprintf(stderr, "semanticická analýza: interní chyba překladače neovlivněná vstupním programem (např. chyba alokace paměti). \n");
        exit(99);
    }

    if ((strcmp(op, "declare")) == 0) {
        htabItem_t* alreadyDefined = htabGet(localHashTable, varName);
        if (alreadyDefined != NULL) {
            fprintf(stderr, "semanticická analýza: nepovolená redefinice funkce, proměnné, typu. \n");
            exit(4);
        }
        htabItem_t *varData = htabWriteNew(localHashTable, varName);
        varData->isMutable = parent->isVar;
        varData->isFunction = false;
        varData->paramCount = 0;
        varData->paramTypes = NULL;
        varData->returnType = parent->dataType;

    } else if ((strcmp(op, "assign")) == 0) {
        htabItem_t* foundItem = NULL;
        htab_t* searchScope = localHashTable;
        while (searchScope != NULL){
            foundItem = htabGet(searchScope, varName);
            if (foundItem != NULL){
                break;
            }
            searchScope = searchScope->parent;     
        }
        htabItem_t* globallyDefined = htabGet(globalHashTable, varName);
        if (foundItem == NULL && globallyDefined == NULL) {
            fprintf(stderr, "semanticická analýza:  použití nedefinované funkce, proměnné nebo typu; chybějící funkce s anotací @main \n");
            exit(3);
        }

        if (foundItem->isMutable == false) {
            fprintf(stderr, "semanticická analýza:  pokus o zápis do nemodifikovatelné proměnné nebo formálního parametru funkce \n");
            exit(7);
        }
    } else {
        fprintf(stderr, "semanticická analýza: interní chyba překladače neovlivněná vstupním programem (např. chyba alokace paměti). \n");
        exit(99);
    }
}

static void checkNode(tnode *parent, htab_t* hashTable, htab_t** localHashTable, int* inBlock, int* currentReturnType) {
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
                fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                exit(6);
            }
            break;

        case opMod:
            if (leftType == TYPE_INT && rightType == TYPE_INT) {
                parent->dataType = TYPE_INT;
            } else {
                fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                exit(6);
            }
            break;

        case opEqual:
        case opLesser:
        case opBigger:
            if (leftType == rightType) {
                parent->dataType = TYPE_BOOL;
            } else {
                fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
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
                fprintf(stderr, "semanticická analýza:  použití nedefinované funkce, proměnné nebo typu; chybějící funkce s anotací @main \n");
                exit(3);
            }
            int isFunction = funcDef->isFunction;
            if (isFunction == 0){
                fprintf(stderr, "semanticická analýza:  použití nedefinované funkce, proměnné nebo typu; chybějící funkce s anotací @main \n");
                exit(3);
            }
            int parCount = funcDef->paramCount;
            int *paramTypes = funcDef->paramTypes;
            while (opArg != NULL){
                if (index >= parCount){
                    fprintf(stderr, "semanticická analýza: nesprávný počet nebo typ argumentů při volání funkce, nepovolené parametry funkce s anotací @main. \n");
                    exit(5);
                } 

                int argType = opArg->left->dataType;
                opArg = opArg->right;
                int expectedType = paramTypes[index];
                if (argType != expectedType){
                    if (expectedType == TYPE_DOUBLE && argType == TYPE_INT){
                        //automaticka konverze z int na double
                    } else {
                        fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                        exit(6);
                    }
                }
                index++;
            }
            if (index != parCount){
                fprintf(stderr, "semanticická analýza: nesprávný počet nebo typ argumentů při volání funkce, nepovolené parametry funkce s anotací @main. \n");
                exit(5);
            }
            parent->dataType = funcDef->returnType;  
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
                fprintf(stderr, "semanticická analýza: nepovolená redefinice funkce, proměnné, typu.");
                exit(4);
            }

            if (leftType != rightType || parent->dataType != leftType) {
                if (leftType == TYPE_DOUBLE && rightType == TYPE_INT){
                    // automaticka konverze
                } 
                else {
                    fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                    exit(6);
                }
            }
            htabItem_t *varData = htabWriteNew(hashTable, varName);
            varData->isFunction = false;
            varData->isMutable = parent->isVar;
            varData->paramCount = 0;
            varData->paramTypes = NULL;
            varData->returnType = parent->dataType;
            break;
        }

        case funcDef:
        {
            *currentReturnType = parent->dataType;
            char *funcName = parent->left->value;
            htabItem_t *funcDef = htabGet(hashTable, funcName);

            if (funcDef != NULL){
                fprintf(stderr, "semanticická analýza: nepovolená redefinice funkce, proměnné, typu.");
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
            htabItem_t *funcData = htabWriteNew(hashTable, funcName);
            funcData->isFunction = true;
            funcData->paramCount = count;
            funcData->paramTypes = typesArray;
            funcData->returnType = parent->dataType;
            funcData->isMutable = false;
            break;
        }

        case opAssign:
        {
            if (leftType != rightType) {
                if (leftType == TYPE_DOUBLE && rightType == TYPE_INT){
                    // automaticka konverze
                } 
                else {
                    fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                    exit(6);
                }
            }

            if (*inBlock) {
                char *varName = parent->left->value;
                blockCheck("assign", varName, parent, hashTable, *localHashTable);
                break;
            }

            char *varName = parent->left->value;

            htabItem_t *varDef = htabGet(hashTable, varName);
            if (varDef == NULL) {
                fprintf(stderr, "semanticická analýza:  použití nedefinované funkce, proměnné nebo typu; chybějící funkce s anotací @main \n");
                exit(3);
            }

            if (varDef->isMutable == false) {
                fprintf(stderr, "semanticická analýza:  pokus o zápis do nemodifikovatelné proměnné nebo formálního parametru funkce \n");
                exit(7);
            }
            break;
        }

        case opBlock:
        {
            *inBlock = 1;
            if (*localHashTable == NULL){
                *localHashTable = htabCreate(503);
                (*localHashTable)->parent = NULL;
            } else {
                htab_t* newScope = htabCreate(503);
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
                fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                exit(6);
            }
            break;
        }
        case opReturn:
        {
            if (leftType != *currentReturnType){
                fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                exit(6);
            }
            if (*currentReturnType == -1){
                fprintf(stderr, "semanticická analýza: nezařazená chyba (např. nepovolený return, nevyčerpávající match nebo nulový krok rozsahu). \n");
                exit(10);
            }
        }
    }
}

static void recursiveSemanticCheck(tnode *ptr, htab_t* hashTable, htab_t** localHashTable, int* inBlock, int* currentReturnType){
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
    htab_t* localHashTable = NULL;
    int currentReturnType = -1;
    recursiveSemanticCheck(ptr, hashTable, &localHashTable, &inBlock, &currentReturnType);
    htabItem_t *mainFunc = htabGet(hashTable, "@main");
    if (mainFunc == NULL) {
        fprintf(stderr, "semanticická analýza:  použití nedefinované funkce, proměnné nebo typu; chybějící funkce s anotací @main \n");
        exit(3);
    }

    if (mainFunc->paramCount > 0){
        fprintf(stderr, "semanticická analýza: nesprávný počet nebo typ argumentů při volání funkce, nepovolené parametry funkce s anotací @main. \n");
        exit(5);
    }
}