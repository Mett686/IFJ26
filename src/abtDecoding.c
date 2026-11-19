#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "htab.h"
#include "abtDecoding.h"
#include "parser.h"


static void blockCheck(const char* op, char* varName, AST_Node* parent, htab_t* globalHashTable, htab_t* localHashTable) {
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
        htabItem_t *varData = htabWriteNew(localHashTable, varName, NULL);
        varData->isMutable = (parent->node_type == NODE_VAR_DEF);
        varData->isFunction = false;
        varData->paramCount = 0;
        varData->paramTypes = NULL;
        varData->returnType = parent->data_type;

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
        htabItem_t *target = (foundItem != NULL) ? foundItem : globallyDefined;
        if (target->isMutable == false) {
            fprintf(stderr, "semanticická analýza:  pokus o zápis do nemodifikovatelné proměnné nebo formálního parametru funkce \n");
            exit(7);
        }
    } else {
        fprintf(stderr, "semanticická analýza: interní chyba překladače neovlivněná vstupním programem (např. chyba alokace paměti). \n");
        exit(99);
    }
}

int isNumericType(int type) {
    return (type == TYPE_INT || type == TYPE_DOUBLE);
}

static void checkNode(AST_Node *parent, htab_t* hashTable, htab_t** localHashTable, int* inBlock, int* currentReturnType, int* mainFuncCount) {
    int leftType;
    int rightType;

    if (parent->left != NULL) {
        leftType = parent->left->data_type;
    } else {
        leftType = -1;
    }

    if (parent->right != NULL) {
        rightType = parent->right->data_type;
    } else {
        rightType = -1;
    }

    switch (parent->node_type) {
        case NODE_EXPRETION:
            switch (parent->operator_type) {
                case OT_PLUS:
                    {
                        if (leftType == TYPE_STRING && rightType == TYPE_STRING) {
                            parent->data_type = TYPE_STRING;
                        } else if (isNumericType(leftType) && isNumericType(rightType)) {
                            parent->data_type = (leftType == TYPE_DOUBLE || rightType == TYPE_DOUBLE) ? TYPE_DOUBLE : TYPE_INT;
                        } else {
                            fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                            exit(6);
                        }
                        break;
                    }
                case OT_MINUS:
                case OT_TIMES:
                case OT_DIVIDE:
                {
                    if (isNumericType(leftType) && isNumericType(rightType)) {
                        parent->data_type = (leftType == TYPE_DOUBLE || rightType == TYPE_DOUBLE) ? TYPE_DOUBLE : TYPE_INT;
                    } else {
                        fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                        exit(6);
                    }
                    break;
                }

                case OT_MOD:
                {
                    if (leftType == TYPE_INT && rightType == TYPE_INT) {
                        parent->data_type = TYPE_INT;
                    } else {
                        fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                        exit(6);
                    }
                    break;
                }    

                case OT_EQUAL:
                case OT_NOT_EQUAL:
                case OT_LESS:
                case OT_GREATER:
                case OT_LESS_EQUAL:
                case OT_GREATER_EQUAL:
                {
                    if (leftType == rightType) {
                        parent->data_type = TYPE_BOOL;
                    } else {
                        fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                        exit(6);
                    }
                    break;
                }
                case OT_AND:
                case OT_OR:
                {
                    if (leftType == TYPE_BOOL && rightType == TYPE_BOOL) {
                        parent->data_type = TYPE_BOOL;
                    } else {
                        fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                        exit(6);
                    }
                    break;
                }      
        }
        break;    

        case NODE_FUNC_CALL:
        {
            char *funcName = parent->left->name;
            AST_Node *opArg = parent->right;
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

                int argType = opArg->left->data_type;
                opArg = opArg->right;
                int expectedType = paramTypes[index];
                if (argType != expectedType){
                    if (expectedType == TYPE_DOUBLE && argType == TYPE_INT){
                        //automaticka konverze z int na double
                    } else {
                        fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                        exit(5);
                    }
                }
                index++;
            }
            if (index != parCount){
                fprintf(stderr, "semanticická analýza: nesprávný počet nebo typ argumentů při volání funkce, nepovolené parametry funkce s anotací @main. \n");
                exit(5);
            }
            parent->data_type = funcDef->returnType;  
            break;
        }

        case NODE_VAR_DEF:
        case NODE_VAL_DEF:
        {
            bool isMutable = (parent->node_type == NODE_VAR_DEF);
            char *varName = parent->left->name;

            if (leftType != rightType || parent->data_type != leftType) {
                if (leftType == TYPE_DOUBLE && rightType == TYPE_INT){
                    // automaticka konverze
                } 
                else {
                    fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                    exit(6);
                }
            }

            if (*inBlock) {
                blockCheck("declare", varName, parent, hashTable, *localHashTable);
                break;
            }

            htabItem_t *varDef = htabGet(hashTable, varName);
            if (varDef != NULL) {
                fprintf(stderr, "semanticická analýza: nepovolená redefinice funkce, proměnné, typu.");
                exit(4);
            }
            

            htabItem_t *varData = htabWriteNew(hashTable, varName, NULL);
            varData->isFunction = false;
            varData->isMutable = isMutable;
            varData->paramCount = 0;
            varData->paramTypes = NULL;
            varData->returnType = parent->data_type;
            break;
        }

        case NODE_FUNC_DEF:
        case NODE_FUNC_DEF_MAIN:
        {
            *currentReturnType = parent->data_type;
            char *funcName = parent->left->name;
            htabItem_t *funcDef = htabGet(hashTable, funcName);

            if (funcDef != NULL){
                fprintf(stderr, "semanticická analýza: nepovolená redefinice funkce, proměnné, typu.");
                exit(4);
            }

            int count = 0;
            AST_Node *paramNode = parent->left; 
    
            while (paramNode != NULL) {
                count++;
                paramNode = paramNode->next;
            }

            if (parent->node_type == NODE_FUNC_DEF_MAIN){
                (*mainFuncCount)++;

                if (*mainFuncCount > 1){
                    fprintf(stderr, "semanticická analýza: nepovolená redefinice funkce, proměnné, typu.");
                    exit(4);
                }

                if (count > 0){
                    fprintf(stderr, "semanticická analýza: nesprávný počet nebo typ argumentů při volání funkce, nepovolené parametry funkce s anotací @main. \n");
                    exit(5);
                }

                if (parent->data_type != TYPE_UNIT){
                    fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                    exit(6);
                }

            }

            

            int *typesArray = NULL;
            if (count > 0) {
                typesArray = malloc(count * sizeof(int));

                if (typesArray == NULL) {
                    fprintf(stderr, "semanticická analýza: interní chyba překladače neovlivněná vstupním programem (např. chyba alokace paměti). \n");
                    exit(99);
                }

                paramNode = parent->left;
                int index = 0;
                while (paramNode != NULL) {
                    typesArray[index] = paramNode->data_type;
                    index++;
                    paramNode = paramNode->next;
                }
            }
            htabItem_t *funcData = htabWriteNew(hashTable, funcName, NULL);
            funcData->isFunction = true;
            funcData->paramCount = count;
            funcData->paramTypes = typesArray;
            funcData->returnType = parent->data_type;
            funcData->isMutable = false;
            break;
        }

        case NODE_ASSIGNMENT:
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
                char *varName = parent->left->name;
                blockCheck("assign", varName, parent, hashTable, *localHashTable);
                break;
            }

            char *varName = parent->left->name;

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

        case NODE_IF_STATEMENT:
        case NODE_WHILE_LOOP:
        {
            if (leftType != TYPE_BOOL){
                fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                exit(6);
            }
            break;
        }
        case NODE_RETURN:
        {
            if (leftType != *currentReturnType){
                if (*currentReturnType == TYPE_DOUBLE && leftType == TYPE_INT) {
                }
                else {
                    fprintf(stderr, "semanticická analýza: chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci \n");
                    exit(6);
                }
                
            }
            if (*currentReturnType == TYPE_UNIT) {
                break;
            }

            if (parent->left == NULL) {
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

static void recursiveSemanticCheck(AST_Node *ptr, htab_t* hashTable, htab_t** localHashTable, int* inBlock, int* currentReturnType, int* mainFuncCount){
    //postOrder browsing through the abstract tree
    if (ptr == NULL) {
        return;
    }

    int prevReturnType = *currentReturnType;
    if (ptr->node_type == NODE_FUNC_DEF || ptr->node_type == NODE_FUNC_DEF_MAIN) {
        *currentReturnType = ptr->data_type;
    }

    if (ptr->node_type == NODE_BLOCK) {
        *inBlock = 1;
        if (*localHashTable == NULL){
            *localHashTable = htabCreate(503, 0);
            (*localHashTable)->parent = NULL;
        } else {
            htab_t* newScope = htabCreate(503, 0);
            newScope->parent = *localHashTable;
            *localHashTable = newScope;
        }          
    }

    recursiveSemanticCheck(ptr->left, hashTable, localHashTable, inBlock, currentReturnType, mainFuncCount);
    recursiveSemanticCheck(ptr->right, hashTable, localHashTable, inBlock, currentReturnType, mainFuncCount);

    if (ptr->left != NULL || ptr->right != NULL) {
        checkNode(ptr, hashTable, localHashTable, inBlock, currentReturnType, mainFuncCount);
    }

    if (ptr->node_type == NODE_BLOCK) {
        if (*localHashTable != NULL) {
            *localHashTable = (*localHashTable)->parent;
        }
        if (*localHashTable == NULL) {
            *inBlock = 0;
        }
    }

    if (ptr->node_type == NODE_FUNC_DEF || ptr->node_type == NODE_FUNC_DEF_MAIN) {
        *currentReturnType = prevReturnType;
    }
    
    recursiveSemanticCheck(ptr->next, hashTable, localHashTable, inBlock, currentReturnType, mainFuncCount);
}

void semanticCheck(AST_Node *ptr, htab_t* hashTable) {
    int inBlock = 0;
    htab_t* localHashTable = NULL;
    int currentReturnType = -1;
    int mainFuncCount = 0;
    recursiveSemanticCheck(ptr, hashTable, &localHashTable, &inBlock, &currentReturnType, &mainFuncCount);

    if (mainFuncCount == 0) {
        fprintf(stderr, "semanticická analýza:  použití nedefinované funkce, proměnné nebo typu; chybějící funkce s anotací @main \n");
        exit(3);
    }
}