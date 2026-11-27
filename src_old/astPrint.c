#include "astPrint.h"

static const char *node_type_to_str(Node_type type) {
    switch (type) {
        case NODE_PROGRAM:        return "NODE_PROGRAM";
        case NODE_DEF_LIST:       return "NODE_DEF_LIST";
        case NODE_FUNC_DEF:       return "NODE_FUNC_DEF";
        case NODE_FUNC_DEF_MAIN:  return "NODE_FUNC_DEF_MAIN";
        case NODE_PARAM:          return "NODE_PARAM";
        case NODE_STATEMENT_LIST: return "NODE_STATEMENT_LIST";
        case NODE_BODY:           return "NODE_BODY";
        case NODE_VAL_DEF:        return "NODE_VAL_DEF";
        case NODE_VAR_DEF:        return "NODE_VAR_DEF";
        case NODE_ASSIGN:         return "NODE_ASSIGN";
        case NODE_FUNC_CALL:      return "NODE_FUNC_CALL";
        case NODE_ARG_LIST:       return "NODE_ARG_LIST";
        case NODE_IF:             return "NODE_IF";
        case NODE_ELSE:           return "NODE_ELSE";
        case NODE_WHILE_LOOP:     return "NODE_WHILE_LOOP";
        case NODE_RETURN:         return "NODE_RETURN";
        case NODE_EXP:            return "NODE_EXP";
        default:                  return "NODE_UNKNOWN";
    }
}

static const char *data_type_to_str(Data_Type type) {
    switch (type) {
        case TYPE_INT:    return "Int";
        case TYPE_DOUBLE: return "Double";
        case TYPE_STRING: return "String";
        case TYPE_UNIT:   return "Unit";
        case TYPE_BOOL:   return "Boolean";
        case TYPE_NULL:
        default:          return NULL;
    }
}

static const char *op_type_to_str(Operator_type op) {
    switch (op) {
        case OT_PUSH:          return "PUSH";
        case OT_PLUS:          return "+";
        case OT_MINUS:         return "-";
        case OT_TIMES:         return "*";
        case OT_DIVIDE:        return "/";
        case OT_UNARY_MINUS:   return "unary -";
        case OT_LESS:          return "<";
        case OT_GREATER:       return ">";
        case OT_EQUAL:         return "==";
        case OT_NOT_EQUAL:     return "!=";
        case OT_LESS_EQUAL:    return "<=";
        case OT_GREATER_EQUAL: return ">=";
        default:               return NULL;
    }
}

static void print_node_details(FILE *out, const AST_Node *node) {
    if (node->name != NULL) {
        fprintf(out, " name=\"%s\"", node->name);
    }

    const char *dtype = data_type_to_str(node->data_type);
    if (dtype != NULL) {
        fprintf(out, " type=%s", dtype);
    }

    const char *op = op_type_to_str(node->operator_type);
    if (op != NULL) {
        fprintf(out, " op=%s", op);
    }

    if (node->node_type == NODE_EXP) {
        if (node->data_type == TYPE_INT) {
            fprintf(out, " val=%lld", node->int_value);
        } else if (node->data_type == TYPE_DOUBLE) {
            fprintf(out, " val=%g", node->double_value);
        } else if (node->data_type == TYPE_STRING && node->string_value != NULL) {
            fprintf(out, " str=\"%s\"", node->string_value);
        } else if (node->data_type == TYPE_BOOL) {
            fprintf(out, " bool=%s", node->bool_value ? "true" : "false");
        }
    }
}

static void ast_print_rec(FILE *out, const AST_Node *node, const char *prefix, bool is_last, const char *edge_label) {
    if (node == NULL) {
        return;
    }

    fprintf(out, "%s%s", prefix, is_last ? "└── " : "├── ");
    if (edge_label != NULL) {
        fprintf(out, "[%s] ", edge_label);
    }
    fprintf(out, "%s", node_type_to_str(node->node_type));
    print_node_details(out, node);
    fprintf(out, "\n");

    char next_prefix[512];
    snprintf(next_prefix, sizeof(next_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    // Zozbierame nenulové vetvy (left, right, next), aby sme vedeli správne určiť poslednú vetvu
    const AST_Node *children[3] = { node->left, node->right, node->next };
    const char *labels[3]       = { "left", "right", "next" };

    int last_idx = -1;
    for (int i = 0; i < 3; i++) {
        if (children[i] != NULL) {
            last_idx = i;
        }
    }

    for (int i = 0; i < 3; i++) {
        if (children[i] != NULL) {
            ast_print_rec(out, children[i], next_prefix, i == last_idx, labels[i]);
        }
    }
}

void ast_print(FILE *out, const AST_Node *root) {
    if (out == NULL) {
        out = stderr;
    }
    if (root == NULL) {
        fprintf(out, "(empty AST)\n");
        return;
    }
    ast_print_rec(out, root, "", true, NULL);
}