
//autor: xmiklej00
#include <stdio.h>
#include <scanner.h>
#include <stdbool.h>

typedef enum{
NODE_PROGRAM,
NODE_DEFINITION_LIST,
NODE_FUNC_DEF,
NODE_FUNC_DEF_MAIN,
NODE_PARAMET,
NODE_TYPE,
NODE_STATEMENT_LIST,
NODE_STATEMENT,
NODE_VAL_DEF,
NODE_VAR_DEF,
NODE_OPT_EOL,
NODE_IF_STATEMENT,
NODE_ELSE_PART,
NODE_WHILE_LOOP,
NODE_OPT_TYPE,
NODE_BLOCK,
NODE_ARGUMENT_LIST,
NODE_BETWEEN,
NODE_RETURN,
NODE_STATEMENT_REST,
NODE_EXPRETION
}Node_type;

typedef enum{
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

typedef struct AST_Node_t{
    Node_type node_type;
    Data_Type data_type;
    AST_Node *next;
    AST_Node *left;
    AST_Node *right;

    char *name;

    int int_value;
    double double_value;
    char*  string_value;
    bool bool_value;

    Operator_type operator_type;

}AST_Node;

typedef enum{
PS_PLUS,
PS_STAR,
PS_SLASH,
PS_EQUAL,
PS_GREATER,
PS_LESS,
PS_LESSEQUAL,
PS_GREATEREQUAL,
PS_NOTEQUAL,
PS_MINUS,
PS_OPENING_PARENTHESES,
PS_CLOSING_PARENTHESES,
PS_END,
PS_ID,
PS_UNARYPLUS,
PS_UNARYMINUS,
PS_COUNT
}Precedens_Symbol;

typedef enum{
P_GREATER,
P_LESS,
P_EQUAL,
P_NONE
}Precedens;

static const char table [PS_COUNT][PS_COUNT] = {
    {'>','<','<','>','>','>','>','>','>','>','<','>','>','<','<','<'},
    {'>','>','>','>','>','>','>','>','>','>','<','>','>','<','<','<'},
    {'>','>','>','>','>','>','>','>','>','>','<','>','>','<','<','<'},
    {'<','<','<',' ','<','<','<','<',' ','<','<','>','>','<','<','<'},
    {'<','<','<','>',' ',' ',' ',' ','>','<','<','>','>','<','<','<'},
    {'<','<','<','>',' ',' ',' ',' ','>','<','<','>','>','<','<','<'},
    {'<','<','<','>',' ',' ',' ',' ','>','<','<','>','>','<','<','<'},
    {'<','<','<','>',' ',' ',' ',' ','>','<','<','>','>','<','<','<'},
    {'<','<','<',' ','<','<','<','<',' ','<','<','>','>','<','<','<'},
    {'>','<','<','>','>','>','>','>','>','>','<','>','>','<','<','<'},
    {'<','<','<','<','<','<','<','<','<','<','<','=',' ','<','<','<'},
    {'>','>','>','>','>','>','>','>','>','>',' ','>','>',' ',' ',' '},
    {'<','<','<','<','<','<','<','<','<','<','<','>',' ','<','<','<'},
    {'>','>','>','>','>','>','>','>','>','>','<','>','>',' ',' ',' '},
    {'>','>','>','>','>','>','>','>','>','>','<','>','>','<','>','>'},
    {'>','>','>','>','>','>','>','>','>','>','<','>','>','<','<','<'},
};


AST_Node* AST_inicial();




