// ast.h
// xmiklej00

typedef enum{
NODE_PROGRAM,
NODE_DEFINITION_LIST,
NODE_FUNC_DEF,
NODE_FUNC_DEF_MAIN,
NODE_PARAMET_LIST,
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
NODE_STATEMENT_REST,
NODE_ARGUMENT_LIST,
NODE_BETWEEN,
NODE_PARAMET_LIST_REST,
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

typedef struct AST_Node{
    Node_type node_type;
    Data_Type data_type;
    AST_Node* next;
    AST_Node* left;
    AST_Node* right;

    char *name;

    int int_value;
    double double_value;
    char*  string_value;
    bool bool_value;

    Operator_type operator_type;
}AST_Node_t;