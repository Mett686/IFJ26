//autor: xmiklej00,
#ifndef AST_H
#define AST_H

#include <stdbool.h>

typedef enum{
NODE_BODY,
NODE_PROGRAM, // --
// left - nejaka def
// right - DEF_LIST
NODE_DEF_LIST, // --
// left - nejaka def
// right - DEF_LIST
NODE_FUNC_DEF, // 
// left - PARAM
// right - STATEMENT_LIST
NODE_FUNC_DEF_MAIN, // 
// left - PARAM
// right - STATEMENT_LIST
NODE_PARAM, // 
// left - PARAM
NODE_STATEMENT_LIST, // -- 
// left - FUNC_CALL, ASSIGN, VAL, VAR, IF, WHILE, EXP, RETURN
// right - STATEMENT_LIST
NODE_STATEMENT, // -- vyuziva pouze synAn
NODE_FUNC_CALL, //
// left - ARG_LIST
NODE_ASSIGN, // -- id
// left - EXP
NODE_VAL_DEF, // -- id
// left - EXP
NODE_VAR_DEF,
// left - EXP
NODE_IF, //
// left - EXP
// right - STATEMENT_LIST
// next - ELSE
NODE_ELSE, //
// left - STATEMENT_LIST
NODE_WHILE_LOOP, //
// left - EXP
// right - STATEMENT_LIST
NODE_ARG_LIST, //
// left - EXP
// right - ARG_LIST
NODE_RETURN,
// left - EXP -- musi dopnovat semAn
NODE_EXP // TODO
}Node_type;

typedef enum{
OT_UNARY_PLUS,
OT_UNARY_MINUS,
OT_PLUS,
OT_MINUS,
OT_DIVIDE,
OT_EQUAL,
OT_NOT_EQUAL,
OT_LESS,
OT_GREATER,
OT_LESS_EQUAL,
OT_GREATER_EQUAL,
OT_TIMES,
OT_AND,
OT_OR
}Operator_type;

typedef enum{
TYPE_INT,
TYPE_DOUBLE,
TYPE_STRING,
TYPE_BOOL,
TYPE_UNIT,
TYPE_NULL
}Data_Type;

typedef struct AST_Node{
    Node_type node_type;
    Data_Type data_type;
    struct AST_Node* next;
    struct AST_Node* left;
    struct AST_Node* right;

    char *name;

    int int_value;
    double double_value;
    char*  string_value;
    bool bool_value;

    Operator_type operator_type;
} AST_Node;

#endif