// Implementace překladače imperativního jazyka IFJ26
// abtDecoding.c
// Sémantická analýza a dekódování abstraktního syntaktického stromu

// Autor: Marek Heckelmoser (xheckem00)

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "htab.h"
#include "abtDecoding.h"
#include "ast.h"

#define TYPE_ANY -2

static bool isNumericType(int type) { return (type == TYPE_INT || type == TYPE_DOUBLE); }

static htabItem_t* blockCheck(const char* name, htab_t* hashTable, htab_t* localHashTable) {
    htab_t* scope = localHashTable;
    while (scope != NULL) {
        htabItem_t* item = htabGet(scope, (char*)name);
        if (item != NULL) {
            return item;
        }
        scope = scope->parent;
    }

    return htabGet(hashTable, (char*)name);
}

static void addBuiltin(htab_t* hashTable, const char* name, int returnType, int paramCount, const int* params) {
    htabItem_t* item = htabWriteNew(hashTable, (char*)name);
    if (item == NULL) {
        exit(99);
    }
    item->isFunction = true;
    item->isMutable = false;
    item->returnType = returnType;
    item->paramCount = paramCount;
    if (paramCount > 0) {
        item->paramTypes = malloc(paramCount * sizeof(int));
        if (item->paramTypes == NULL)
            exit(99);
        memcpy(item->paramTypes, params, paramCount * sizeof(int));
    } else {
        item->paramTypes = NULL;
    }
}

static void registerBuiltins(htab_t* hashTable) {
    int pAny[] = {TYPE_ANY};
    int pDouble[] = {TYPE_DOUBLE};
    int pString[] = {TYPE_STRING};
    int pSubstr[] = {TYPE_STRING, TYPE_INT, TYPE_INT};
    int pOrd[] = {TYPE_STRING, TYPE_INT};
    int pInt[] = {TYPE_INT};

    addBuiltin(hashTable, "print", TYPE_UNIT, 1, pAny);
    addBuiltin(hashTable, "readLine", TYPE_STRING, 0, NULL);
    addBuiltin(hashTable, "readInt", TYPE_INT, 0, NULL);
    addBuiltin(hashTable, "readDouble", TYPE_DOUBLE, 0, NULL);
    addBuiltin(hashTable, "toStr", TYPE_STRING, 1, pAny);
    addBuiltin(hashTable, "toInt", TYPE_INT, 1, pDouble);
    addBuiltin(hashTable, "strLength", TYPE_INT, 1, pString);
    addBuiltin(hashTable, "subStr", TYPE_STRING, 3, pSubstr);
    addBuiltin(hashTable, "ord", TYPE_INT, 2, pOrd);
    addBuiltin(hashTable, "chr", TYPE_STRING, 1, pInt);
}

static void registerUserFunctions(AST_Node* ptr, htab_t* hashTable, int* mainFuncCount) {
    AST_Node* curr = ptr;
    while (curr != NULL) {
        AST_Node* def = curr->left;
        if (def != NULL && (def->node_type == NODE_FUNC_DEF || def->node_type == NODE_FUNC_DEF_MAIN)) {
            char* funcName = def->name;
            if (htabGet(hashTable, funcName) != NULL) {
                fprintf(stderr, "semantická analýza: nepovolená redefinice funkce '%s'.\n", funcName);
                exit(4);
            }

            int count = 0;
            AST_Node* param = def->left;
            while (param != NULL && param->name != NULL) {
                count++;
                param = param->left;
            }

            if (def->node_type == NODE_FUNC_DEF_MAIN) {
                (*mainFuncCount)++;
                if (*mainFuncCount > 1) {
                    fprintf(stderr, "semantická analýza: více funkcí s anotací @main.\n");
                    exit(4);
                }
                if (count > 0) {
                    fprintf(stderr, "semantická analýza: funkce @main nesmí mít parametry.\n");
                    exit(5);
                }
                if (def->data_type != TYPE_UNIT) {
                    fprintf(stderr, "semantická analýza: funkce @main musí mít návratový typ Unit.\n");
                    exit(6);
                }
            }

            int* typesArray = NULL;
            if (count > 0) {
                typesArray = malloc(count * sizeof(int));
                if (typesArray == NULL)
                    exit(99);
                param = def->left;
                for (int i = 0; i < count; i++) {
                    typesArray[i] = param->data_type;
                    param = param->left;
                }
            }

            htabItem_t* funcData = htabWriteNew(hashTable, funcName);
            if (funcData == NULL)
                exit(99);
            funcData->isFunction = true;
            funcData->isMutable = false;
            funcData->paramCount = count;
            funcData->paramTypes = typesArray;
            funcData->returnType = def->data_type;
            def->symbol = funcData;
        }
        curr = curr->right;
    }
}

static AST_Node* wrapWithNode(Node_type ntype, AST_Node* child) {
    if (child == NULL) {
        return NULL;
    }

    AST_Node* conv = calloc(1, sizeof(AST_Node));
    if (conv == NULL) {
        fprintf(stderr, "semantická analýza: selhala alokace uzlu konverze.\n");
        exit(99);
    }

    conv->node_type = ntype;
    if (ntype == NODE_EXP) {
        conv->operator_type = OT_CONV;
        conv->data_type = TYPE_DOUBLE;
    } else {
        conv->data_type = child->data_type;
    }
    conv->left = child;
    conv->right = NULL;
    conv->next = NULL;

    return conv;
}

static AST_Node* wrapLastExprInReturn(AST_Node* body) {
    if (body == NULL) {
        return NULL;
    }

    if (body->next == NULL) {
        if (body->node_type == NODE_RETURN) {
            return body;
        }
        return wrapWithNode(NODE_RETURN, body);
    }

    AST_Node* curr = body;
    while (curr->next != NULL && curr->next->next != NULL) {
        curr = curr->next;
    }

    if (curr->next != NULL) {
        curr->next = wrapWithNode(NODE_RETURN, curr->next);
    }
    return body;
}

static void checkNode(AST_Node* parent, htab_t* hashTable, htab_t* localHashTable, int currentReturnType) {
    int leftType = (parent->left != NULL) ? (int)parent->left->data_type : -1;
    int rightType = (parent->right != NULL) ? (int)parent->right->data_type : -1;

    switch (parent->node_type) {
    case NODE_EXP:
        if (parent->left == NULL && parent->right == NULL) {
            if (parent->name != NULL) {
                htabItem_t* var = blockCheck(parent->name, hashTable, localHashTable);
                parent->symbol = var;
                if (var == NULL || var->isFunction) {
                    fprintf(stderr, "semantická analýza: použití nedefinované proměnné '%s'.\n", parent->name);
                    exit(3);
                }
                parent->data_type = var->returnType;
            }
            break;
        }

        switch (parent->operator_type) {
        case OT_PLUS:
            if (leftType == TYPE_STRING && rightType == TYPE_STRING) {
                // pretypovani OT_PLUS na OT_CONCAT
                parent->operator_type = OT_CONCAT;
                parent->data_type = TYPE_STRING;
            } else if (isNumericType(leftType) && isNumericType(rightType)) {
                parent->data_type = (leftType == TYPE_DOUBLE || rightType == TYPE_DOUBLE) ? TYPE_DOUBLE : TYPE_INT;
                if (parent->data_type == TYPE_DOUBLE) {
                    if (leftType == TYPE_INT) {
                        parent->left = wrapWithNode(NODE_EXP, parent->left);
                    } else if (rightType == TYPE_INT) {
                        parent->right = wrapWithNode(NODE_EXP, parent->right);
                    }
                }
            } else {
                fprintf(stderr, "semantická analýza: nekompatibilní typy u operátoru '+'.\n");
                exit(6);
            }
            break;

        case OT_MINUS:
        case OT_TIMES:
        case OT_DIVIDE:
            if (isNumericType(leftType) && isNumericType(rightType)) {
                if (leftType == TYPE_DOUBLE || rightType == TYPE_DOUBLE) {
                    parent->data_type = TYPE_DOUBLE;
                } else {
                    parent->data_type = TYPE_INT;
                }
                if (leftType == TYPE_INT && rightType == TYPE_INT && parent->operator_type == OT_DIVIDE) {
                    // pretypovani divide na idivide
                    parent->operator_type = OT_IDIVIDE;
                } else if (leftType != rightType) {
                    if (leftType == TYPE_INT) {
                        parent->left = wrapWithNode(NODE_EXP, parent->left);
                    } else if (rightType == TYPE_INT) {
                        parent->right = wrapWithNode(NODE_EXP, parent->right);
                    }
                }
            } else {
                fprintf(stderr, "semantická analýza: nekompatibilní typy v aritmetickém výrazu.\n");
                exit(6);
            }
            break;

        case OT_LESS:
        case OT_GREATER:
        case OT_LESS_EQUAL:
        case OT_GREATER_EQUAL:
            if ((isNumericType(leftType) && isNumericType(rightType)) ||
                (leftType == TYPE_STRING && rightType == TYPE_STRING)) {
                parent->data_type = TYPE_BOOL;
            } else {
                fprintf(stderr, "semantická analýza: nekompatibilní typy u relačního operátoru.\n");
                exit(6);
            }
            break;

        case OT_EQUAL:
        case OT_NOT_EQUAL:
            if (leftType == rightType || (isNumericType(leftType) && isNumericType(rightType))) {
                parent->data_type = TYPE_BOOL;
            } else {
                fprintf(stderr, "semantická analýza: nekompatibilní typy u porovnání rovnosti.\n");
                exit(6);
            }
            break;

        case OT_AND:
        case OT_OR: {
            if (leftType == TYPE_BOOL && rightType == TYPE_BOOL) {
                parent->data_type = TYPE_BOOL;
            } else {
                fprintf(stderr, "semantická analýza: logické operátory vyžadují Boolean.\n");
                exit(6);
            }
            break;
        }
        case OT_CONCAT:
        case OT_IDIVIDE:
        case OT_CONV:
        case OT_UNARY_PLUS:
        case OT_UNARY_MINUS:
        case OT_PUSH:
            break;
        }

        break;

    case NODE_FUNC_CALL: {
        char* funcName = parent->name;
        htabItem_t* funcDef = htabGet(hashTable, funcName);
        parent->symbol = funcDef;
        if (funcDef == NULL || !funcDef->isFunction) {
            fprintf(stderr, "semantická analýza: volání nedefinované funkce '%s'.\n", funcName);
            exit(3);
        }

        AST_Node* argNode = parent->left;
        int index = 0;
        while (argNode != NULL && argNode->left != NULL) {
            if (index >= funcDef->paramCount) {
                fprintf(stderr, "semantická analýza: příliš mnoho argumentů při volání '%s'.\n", funcName);
                exit(5);
            }
            int argType = argNode->left->data_type;
            int expectedType = funcDef->paramTypes[index];

            if (expectedType != TYPE_ANY && argType != expectedType) {
                if (!(expectedType == TYPE_DOUBLE && argType == TYPE_INT)) {
                    fprintf(stderr, "semantická analýza: chybný typ argumentu při volání '%s'.\n", funcName);
                    exit(5);
                } else {
                    argNode->left = wrapWithNode(NODE_EXP, argNode->left);
                    // konverze int na double
                }
            }
            index++;
            argNode = argNode->right;
        }

        if (index != funcDef->paramCount) {
            fprintf(stderr, "semantická analýza: nesprávný počet argumentů při volání '%s'.\n", funcName);
            exit(5);
        }
        parent->data_type = funcDef->returnType;
        break;
    }

    case NODE_VAR_DEF:
    case NODE_VAL_DEF: {
        bool isMutable = (parent->node_type == NODE_VAR_DEF);
        char* varName = parent->name;
        int initType = leftType;

        if (initType < 0 || initType == TYPE_NULL) {
            fprintf(stderr, "semantická analýza: nelze odvodit typ proměnné '%s'.\n", varName);
            exit(6);
        }

        if (parent->data_type == TYPE_NULL) {
            parent->data_type = initType;
        } else if ((int)parent->data_type != initType) {
            if (!(parent->data_type == TYPE_DOUBLE && initType == TYPE_INT)) {
                fprintf(stderr, "semantická analýza: nekompatibilní typ při inicializaci '%s'.\n", varName);
                exit(6);
            } else {
                parent->left = wrapWithNode(NODE_EXP, parent->left);
                // konverze int na double
            }
        }

        htab_t* targetTable = (localHashTable != NULL) ? localHashTable : hashTable;
        if (htabGet(targetTable, varName) != NULL) {
            fprintf(stderr, "semantická analýza: redefinice proměnné '%s'.\n", varName);
            exit(4);
        }

        htabItem_t* varData = htabWriteNew(targetTable, varName);
        if (varData == NULL)
            exit(99);
        varData->isFunction = false;
        varData->isMutable = isMutable;
        varData->paramCount = 0;
        varData->paramTypes = NULL;
        varData->returnType = parent->data_type;
        parent->symbol = varData;
        break;
    }

    case NODE_ASSIGN: {
        char* varName = parent->name;
        int exprType = leftType;

        htabItem_t* target = blockCheck(varName, hashTable, localHashTable);
        parent->symbol = target;
        if (target == NULL || target->isFunction) {
            fprintf(stderr, "semantická analýza: přiřazení do nedefinované proměnné '%s'.\n", varName);
            exit(3);
        }
        if (!target->isMutable) {
            fprintf(stderr, "semantická analýza: pokus o zápis do nemodifikovatelné proměnné '%s'.\n", varName);
            exit(7);
        }
        if (target->returnType != exprType) {
            if (!(target->returnType == TYPE_DOUBLE && exprType == TYPE_INT)) {
                fprintf(stderr, "semantická analýza: nekompatibilní typ při přiřazení do '%s'.\n", varName);
                exit(6);
            } else {
                parent->left = wrapWithNode(NODE_EXP, parent->left);
            }
        }
        break;
    }

    case NODE_IF:
    case NODE_WHILE_LOOP:
        if (leftType != TYPE_BOOL) {
            fprintf(stderr, "semantická analýza: podmínka v if/while musí být typu Boolean.\n");
            exit(6);
        }
        break;

    case NODE_RETURN:
        if (currentReturnType == -1) {
            fprintf(stderr, "semantická analýza: nepovolený return mimo funkci.\n");
            exit(10);
        }
        if (currentReturnType == TYPE_NULL) {
            fprintf(stderr, "semantická analýza: funkce obsahující return musí mít explicitní návratový typ.\n");
            exit(6);
        }
        if (currentReturnType == TYPE_UNIT) {
            if (parent->left != NULL) {
                fprintf(stderr, "semantická analýza: samostatný return je povolen jen ve funkci typu Unit.\n");
                exit(6);
            }
            break;
        }
        if (parent->left == NULL) {
            fprintf(stderr, "semantická analýza: samostatný return je povolen jen ve funkci typu Unit.\n");
            exit(6);
        }
        if (leftType != currentReturnType) {
            if (!(currentReturnType == TYPE_DOUBLE && leftType == TYPE_INT)) {
                fprintf(stderr, "semantická analýza: nekompatibilní návratový typ v return.\n");
                exit(6);
            } else {
                // konverze int na double
                parent->left = wrapWithNode(NODE_EXP, parent->left);
            }
        }
        break;

    default:
        break;
    }
}

static void recursiveSemanticCheck(AST_Node* ptr, htab_t* hashTable, htab_t* localHashTable, int currentReturnType) {
    if (ptr == NULL)
        return;

    switch (ptr->node_type) {
    case NODE_FUNC_DEF:
    case NODE_FUNC_DEF_MAIN: {
        htab_t* funcScope = htabCreate(503);
        if (funcScope == NULL)
            exit(99);
        funcScope->parent = localHashTable;

        AST_Node* param = ptr->left;
        while (param != NULL && param->name != NULL) {
            if (htabGet(funcScope, param->name) != NULL) {
                fprintf(stderr, "semantická analýza: duplicitní název parametru '%s'.\n", param->name);
                exit(4);
            }
            htabItem_t* pItem = htabWriteNew(funcScope, param->name);
            if (pItem == NULL)
                exit(99);
            param->symbol = pItem;
            pItem->isFunction = false;
            pItem->isMutable = false;
            pItem->paramCount = 0;
            pItem->paramTypes = NULL;
            pItem->returnType = param->data_type;
            param = param->left;
        }

        recursiveSemanticCheck(ptr->right, hashTable, funcScope, ptr->data_type);
        ptr->right = wrapLastExprInReturn(ptr->right);
        break;
    }

    case NODE_IF: {
        recursiveSemanticCheck(ptr->left, hashTable, localHashTable, currentReturnType);
        checkNode(ptr, hashTable, localHashTable, currentReturnType);

        htab_t* thenScope = htabCreate(503);
        if (thenScope == NULL)
            exit(99);
        thenScope->parent = localHashTable;
        recursiveSemanticCheck(ptr->right, hashTable, thenScope, currentReturnType);

        recursiveSemanticCheck(ptr->next, hashTable, localHashTable, currentReturnType);
        break;
    }

    case NODE_ELSE: {
        htab_t* elseScope = htabCreate(503);
        if (elseScope == NULL)
            exit(99);
        elseScope->parent = localHashTable;
        recursiveSemanticCheck(ptr->left, hashTable, elseScope, currentReturnType);
        break;
    }

    case NODE_WHILE_LOOP: {
        recursiveSemanticCheck(ptr->left, hashTable, localHashTable, currentReturnType);
        checkNode(ptr, hashTable, localHashTable, currentReturnType);

        htab_t* whileScope = htabCreate(503);
        if (whileScope == NULL)
            exit(99);
        whileScope->parent = localHashTable;
        recursiveSemanticCheck(ptr->right, hashTable, whileScope, currentReturnType);
        break;
    }

    default:
        recursiveSemanticCheck(ptr->left, hashTable, localHashTable, currentReturnType);
        recursiveSemanticCheck(ptr->right, hashTable, localHashTable, currentReturnType);
        checkNode(ptr, hashTable, localHashTable, currentReturnType);
        recursiveSemanticCheck(ptr->next, hashTable, localHashTable, currentReturnType);
        break;
    }
}

void semanticCheck(AST_Node* ptr, htab_t* hashTable) {
    if (hashTable == NULL) {
        fprintf(stderr, "semantická analýza: interní chyba (NULL tabulka).\n");
        exit(99);
    }

    int mainFuncCount = 0;
    registerBuiltins(hashTable);
    registerUserFunctions(ptr, hashTable, &mainFuncCount);

    if (mainFuncCount == 0) {
        fprintf(stderr, "semantická analýza: chybějící funkce s anotací @main.\n");
        exit(3);
    }

    recursiveSemanticCheck(ptr, hashTable, NULL, -1);
}