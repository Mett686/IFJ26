//autor: xmiklej00
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "parser.h"
#include "scanner.h"
#include "stack.h"

static void parser_exit(int code, const char *file, int line)
{
    fprintf(stderr, "parser: fatal error %d at %s:%d\n", code, file, line);
    exit(code);
}

#define exit(code) parser_exit(code, __FILE__, __LINE__)

AST_Node* AST_inicial(void){
    AST_Node *table = malloc(sizeof(AST_Node));
    if(table==NULL){
        free(table);
        exit(99);
    }
    table->left =NULL;
    table->right = NULL;
    table->next = NULL;
    table->node_type = NODE_PROGRAM;

    return table;
}

AST_Node* create_node(Node_type type){
    AST_Node *node = malloc(sizeof(AST_Node));
    if(node==NULL){
        exit(99);
    }
    node->node_type=type;
    node->left=NULL;
    node->right=NULL;
    node->next=NULL;
    
    return node;
}

AST_Node* parser(FILE* stream,htab_t *table){
    AST_Node *tree = AST_inicial();
    token_t token = getToken(stream);
    
    while(TKN_NEWLINE == token.type){
        token=getToken(stream);
    };


    if(token.type == TKN_IMPORT){
        parser_prolog(stream,&token);
        token= getToken(stream);

        if(token.type == TKN_DEF || token.type == TKN_AT_MAIN){
            tree = parser_definitionList(stream,&token,tree,table);
        }else{
            //ERROR
            exit(2);
        }
    }
    else {
        //TODO ERROR
        exit(2);
    }
    return tree;
}

void parser_prolog(FILE* stream,token_t *token ){
    
    *token = getToken(stream);
    if(token->type == TKN_IFJ26dot_asterisk){

        *token = getToken(stream);
        if (token->type == TKN_NEWLINE){

        }
    
    }
    else{
        //TODO ERROR
        exit(2);
    }

}

AST_Node* parser_definitionList(FILE* stream ,token_t *token,AST_Node *tree,htab_t *table){
    AST_Node *head = tree;
    while(token->type != TKN_EOF){
        

        if(token->type == TKN_DEF){
            tree->left = create_node(NODE_FUNC_DEF);
            parser_defFunction(stream,token,tree->left,table);

        }else if(token->type == TKN_AT_MAIN){
            tree->left = create_node(NODE_FUNC_DEF_MAIN);
            parser_defFunction(stream,token,tree->left,table);
            parser_defMainFunc(stream,token,tree->left);

            
    
        }else{//TODO ERROR
            exit(2);
        }

        tree->right = create_node(NODE_DEF_LIST);
        tree=tree->right;
        *token= getToken(stream);
            
    }

        return head;
}

void parser_defFunction(FILE*stream,token_t *token,AST_Node*node,htab_t *table){

     *token = getToken(stream);

    if(token->type == TKN_ID){
        node->name = token->string;
        table = htabWriteNew(table,token->string);
        *token = getToken(stream);

        if(token->type == TKN_OPENING_PARENTHESES){
            node->left = create_node(NODE_PARAM);
            parser_parameterList(stream,token,node->left);
            
            
            if (token->type == TKN_CLOSING_PARENTHESES){
                *token = getToken(stream);
            
                if(token->type ==TKN_COLON ){
                    
                    parser_type(stream,token,node);
                    *token = getToken(stream);

                    if (token->type == TKN_ASSING){
                        node->right = create_node(NODE_STATEMENT_LIST);
                        parser_body(stream,token,node->right);
                    }else{
                        exit(2);
                    }
                }
                else if(token->type == TKN_ASSING){
                    node->data_type=TYPE_NULL;
                    node->right = create_node(NODE_STATEMENT_LIST);
                    parser_body(stream,token,node->right);
                
                }else{
                    exit(2);
                    //ERROR
                }             
            }else{
                exit(2);
                //ERROR
            }
        }else{
            exit(2);
        }
    }else{
        exit(2);
        //TODO ERROR
    }
    

}

void parser_defMainFunc(FILE* stream,token_t *token,AST_Node *node){
    *token = getToken(stream);

    if (token->type == TKN_NEWLINE){
        *token = getToken(stream);
        
        if(token->type == TKN_DEF){
        *token = getToken(stream);

            if(token->type == TKN_ID){
                node->name=token->string;
                *token=getToken(stream);

                if(token->type == TKN_OPENING_PARENTHESES){
                    node->left = create_node(NODE_PARAM);
                    parser_parameterList(stream,token,node->left);
                    

                    if(token->type == TKN_CLOSING_PARENTHESES){
                        *token = getToken(stream);

                        if(token->type == TKN_COLON){
                            parser_type(stream,token,node);
                            
                            *token = getToken(stream);

                            if(token->type == TKN_ASSING){
                                node->right = create_node(NODE_STATEMENT_LIST);
                                parser_body(stream,token,node->right);
                            }else{
                                exit(2);
                            }

                        }else if (token->type == TKN_ASSING){
                            node->data_type=TYPE_NULL;
                            node->right = create_node(NODE_STATEMENT_LIST);
                            parser_body(stream,token, node->right);
                        }else{
                            exit(2);
                            //ERROR
                        }
                        
                    }else{
                        exit(2);
                        //ERROR
                    }

                }else{
                    exit(2);
                    //ERROR
                }
            }else{
                exit(2);
                //ERROR
            }

        }else{
            exit(2);
            //ERROR
        }
    }
    else if(token->type == TKN_DEF){
        *token = getToken(stream);



        if(token->type == TKN_ID){
            node->name = token->string;
            *token=getToken(stream);

            if(token->type == TKN_OPENING_PARENTHESES){
                node->left = create_node(NODE_PARAM);
                parser_parameterList(stream,token,node->left);
                

                if(token->type == TKN_CLOSING_PARENTHESES){
                    *token = getToken(stream);

                    if(token->type == TKN_COLON){
                        parser_type(stream,token,node);
                            
                        *token = getToken(stream);

                        if(token->type == TKN_ASSING){
                            node->right = create_node(NODE_STATEMENT_LIST);
                            parser_body(stream,token,node->right);
                        }else{
                            exit(2);
                        }

                    }else if(token->type == TKN_ASSING){
                        node->data_type=TYPE_NULL;
                        node->right = create_node(NODE_STATEMENT_LIST);
                        parser_body(stream,token,node->right);
                    }else{
                        exit(2);
                        //ERROR
                    }
                        
                }else{
                    exit(2);
                    //ERROR
                }

            }else{
                exit(2);
                //ERROR
            }
        }else{
            exit(2);
            //ERROR
        }
    }
    else{
        exit(2);
        //TODO ERROR
    }

}

void parser_type(FILE*stream,token_t *token,AST_Node* node){
    *token = getToken(stream);

    switch (token->type){
        case TKN_INT_KEYWORD:
            node->data_type=TYPE_INT;
            break;
        case TKN_DOUBLE_KEYWORD:
            node->data_type=TYPE_DOUBLE;
            break;
        case TKN_UNIT_KEYWORD:
            node->data_type=TYPE_UNIT;
            break;
        case TKN_STRING_KEYWORD:
            node->data_type=TYPE_STRING;
            break;
        default:
        exit(2);
        //TODO ERROR
        break;
    }
        
    

}

void parser_body(FILE*stream,token_t *token,AST_Node*node){
    *token = getToken(stream);
    switch (token->type)
    {
    case TKN_NEWLINE:
        parser_block(stream,token,node);
        break;
    case TKN_ID:    
        parser_statementRest(stream,token,node);
        *token = getToken(stream);
        if(token->type != TKN_NEWLINE){
            exit(2);//TODO ERROR
        }
        break;
    /*case TKN_IF:
        parser_ifstatement
        break;*/
    case TKN_MINUS:
    case TKN_OPENING_PARENTHESES:
    case TKN_INT:
    case TKN_DOUBLE:
    case TKN_UNIT:
    case TKN_STRING:
        parser_expression(stream,token,node);
        *token = getToken(stream);
        if(token->type != TKN_NEWLINE){
            exit(2);//TODO ERROR
        }
        break;
    default:
    exit(2);
        //TODO ERROR
        break;
    }
    
}

void parser_block(FILE* stream,token_t *token,AST_Node*node){
    *token = getToken(stream);
    if (token->type == TKN_INDENTATION){
        node->left = create_node(NODE_BODY);
        parser_statementList(stream,token,node->left);
        *token = getToken(stream);
    }else{
        exit(2);
        //TODO ERROR
    }
}
    

void parser_statementList(FILE* stream,token_t *token,AST_Node*node){
    *token = getToken(stream);
    while(token->type != TKN_DEDENTATION && token->type != TKN_EOF){
        switch (token->type){
            case TKN_VAL:
                node->left = create_node(NODE_VAL_DEF);
                parser_valDef(stream,token,node->left);
                parser_between(stream,token);
            break;
            case TKN_VAR:
                node->left = create_node(NODE_VAR_DEF);
                parser_varDef(stream,token,node->left);
                parser_between(stream,token);
            break;
            case TKN_IF:
                node->left = create_node(NODE_IF);
                parser_ifStatement(stream,token,node->left);
            break;
            case TKN_WHILE:
                node->left = create_node(NODE_WHILE_LOOP);
                parser_whileLoop(stream,token,node->left);
            break;
            case TKN_ID:
                node->left = create_node(NODE_STATEMENT);
                node->left->name=token->string;
                parser_statementRest(stream,token,node->left);
                parser_between(stream,token);
            break;
            case TKN_INT:
                node->left = create_node(NODE_EXP);
                node->data_type=TYPE_INT;
                parser_expression(stream,token,node->left);
                parser_between(stream,token);
                break;
            case TKN_DOUBLE:
                node->left = create_node(NODE_EXP);
                node->data_type=TYPE_DOUBLE;
                parser_expression(stream,token,node->left);
                parser_between(stream,token);
                break;
            case TKN_UNIT:
                node->left = create_node(NODE_EXP);
                node->data_type=TYPE_UNIT;
                parser_expression(stream,token,node->left);
                parser_between(stream,token);
                break;
            case TKN_STRING:
                node->left = create_node(NODE_EXP);
                node->data_type=TYPE_STRING;
                parser_expression(stream,token,node->left);
                parser_between(stream,token);
            break;
            case TKN_OPENING_PARENTHESES:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                parser_between(stream,token);
            break;
            case TKN_RETURN:
                node->left = create_node(NODE_RETURN);
                parser_returnRest(stream,token,node->left);
                parser_between(stream,token);
            break;
            default:
            exit(2);
                //TODO ERROR
            break;
        } 
        node->right = create_node(NODE_STATEMENT_LIST);
        node=node->right;
        *token = getToken(stream);
    }
}

void parser_between(FILE* stream,token_t *token){
    switch (token->type){

        case TKN_NEWLINE:
        break;
        case TKN_SEMICOLON:
        break;
        case TKN_DEDENTATION:
        case TKN_EOF:
        break;
        default:
        exit(2);
        //TODO ERROR
        break;
    }
}

void parser_valDef(FILE*stream,token_t *token,AST_Node *node){
    *token = getToken(stream);
    if (token->type == TKN_ID){
        node->name = token->string;
        parser_optType(stream,token,node);
        *token = getToken(stream);
        if(token->type == TKN_ASSING){
            node->right = create_node(NODE_EXP);
            parser_expression(stream,token,node->right);

        }else{
            exit(2);
            //ERROR
        }
    }else{
        exit(2);
    //TODO ERROR
    }
}

void parser_optType(FILE* stream,token_t *token,AST_Node *node){
    if(token->type == TKN_COLON){
        parser_type(stream,token,node);
    }
}

void parser_varDef (FILE* stream,token_t *token,AST_Node *node){
    *token = getToken(stream);
    if (token->type == TKN_ID){

        node->name = token->string;
        parser_optType(stream,token,node);
        *token = getToken(stream);

        if(token->type == TKN_ASSING){
            node->right = create_node(NODE_EXP);
            parser_expression(stream,token,node->right);
        
        }else{
            exit(2);
            //ERROR
        }
    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_ifStatement (FILE* stream,token_t *token,AST_Node *node){
    *token = getToken(stream);
    node->left = create_node(NODE_EXP);
    parser_expression(stream,token,node->left);
    
    if (token->type == TKN_THEN){
        node->right = create_node(NODE_STATEMENT_LIST);
        parser_body(stream,token,node->right);
        node->next = create_node(NODE_ELSE);
        parser_elsePart(stream,token,node->next);
    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_elsePart(FILE* stream,token_t *token,AST_Node*node){
    *token= getToken(stream);
    switch(token->type){
        case TKN_ELSE:
            node->left = create_node(NODE_STATEMENT_LIST);
            parser_body(stream,token,node->left);
        break;
        /*BOOLthen  case TKN_NEWLINE:
        case TKN_VAL:
        case TKN_VAR:
        case TKN_IF:
        case TKN_WHILE:
        case TKN_ID:
        case TKN_MINUS:
        case TKN_DEDENTATION:
        case TKN_INT:
        case TKN_DOUBLE:
        case TKN_UNIT:
        case TKN_STRING:
        case TKN_OPENING_PARENTHESES:
        case TKN_RETURN:
        case TKN_EOF:
        break;*/
        default:
        exit(2);
        //TODO ERROR
        break;
    }

}

void parser_whileLoop(FILE* stream,token_t *token,AST_Node *node){
    *token = getToken(stream);
    node->left = create_node(NODE_EXP);
    parser_expression(stream,token,node->left);
    
    if(token->type == TKN_DO){
        node->right = create_node(NODE_STATEMENT_LIST);
        parser_body(stream,token,node->right);
    }else{
        exit(2);
        //TODO ERROR
    }

}

void parser_returnRest(FILE* stream,token_t *token,AST_Node*node){
    *token = getToken(stream);
    switch (token->type){
        case TKN_ID:
            node->left = create_node(NODE_EXP);
            node->left->name = token->string;
            parser_expression(stream,token,node->left);
            break;
        case TKN_MINUS:
            node->left = create_node(NODE_EXP);
            parser_expression(stream,token,node->left);
            break;
        case TKN_INT:
            node->left = create_node(NODE_EXP);
            node->left->data_type = TYPE_UNIT;
            parser_expression(stream,token,node->left);
            break;
        case TKN_DOUBLE:
            node->left = create_node(NODE_EXP);
            node->left->data_type = TYPE_UNIT;
            parser_expression(stream,token,node->left);
            break;
        case TKN_STRING:
            node->left = create_node(NODE_EXP);
            node->left->data_type = TYPE_UNIT;
            parser_expression(stream,token,node->left);
            break;
        case TKN_UNIT:
            node->left = create_node(NODE_EXP);
            node->left->data_type = TYPE_UNIT;
            parser_expression(stream,token,node->left);
            break;

        case TKN_OPENING_PARENTHESES:
            node->left = create_node(NODE_EXP);
            parser_expression(stream,token,node->left);
        break;
        case TKN_NEWLINE:
        case TKN_DEDENTATION:
        case TKN_SEMICOLON:
        case TKN_EOF:
        break;
        default:
        exit(2);
        //TODO ERROR
        break;
    }
}

void parser_parameterList(FILE*stream,token_t *token,AST_Node*node){
    
    *token = getToken(stream);
    
    if(token->type == TKN_ID){
        node->name= token->string;
        
        parser_parameter(stream,token,node);
        parser_parameterListRest(stream,token,node->left);
    }
    else if(token->type == TKN_CLOSING_PARENTHESES){

    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_parameter(FILE*stream,token_t *token,AST_Node *node){
    //TODO ULOZENIE ID DO TABULKY
    *token =getToken(stream);
    if(token->type == TKN_COLON){
        parser_type(stream,token,node);
    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_parameterListRest(FILE*stream,token_t *token,AST_Node *node){
    *token = getToken(stream);

    while(token->type!=TKN_CLOSING_PARENTHESES){

        if(token->type == TKN_COMMA){
            *token = getToken(stream);
            if(token->type==TKN_ID){
                node->left = create_node(NODE_PARAM);
                node = node->left;
                node->name=token->string;
                parser_parameter(stream,token,node);
            }else{
                exit(2);
                //TODO ERROR
            }
        }else{
            exit(2);
            //TODO ERROR
            break;
        }

        node=node->left;

        *token = getToken(stream);
    }
}

void parser_statementRest(FILE*stream,token_t *token,AST_Node*node){
    if(token->type == TKN_ID){
        *token = getToken(stream);
        switch(token->type){
            case TKN_MINUS:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                break;
            case TKN_DIVISION_SLASH:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                break;
            case TKN_TIMES:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                break;
            case TKN_NOT_EQUAL:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                break;
            case TKN_EQUAL:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                break;
            case TKN_LESS:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                break;
            case TKN_LESS_EQUAL:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                break;
            case TKN_GREATER:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                break;
            case TKN_GREATER_EQUAL:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                break;
            case TKN_PLUS:
                node->left = create_node(NODE_EXP);
                parser_expression(stream,token,node->left);
                break;
            case TKN_OPENING_PARENTHESES:
                node->left = create_node(NODE_ARG_LIST);
                parser_argumentList(stream,token,node->left);
                break;
            case TKN_NEWLINE:
            case TKN_DEDENTATION:
            case TKN_CLOSING_PARENTHESES:
            case TKN_SEMICOLON:
            case TKN_EOF:
            //EPSILON
                break;
            default:
            exit(2);
            //TODO ERROR
            break;

        }
    }
}

void parser_argumentList(FILE*stream,token_t *token,AST_Node *node){
    *token = getToken(stream);

    switch (token->type){
        case TKN_ID:
            node->name=token->string;
            parser_expression(stream,token,node);
            node->right = create_node(NODE_ARG_LIST);
            parser_argumentListRest(stream,token,node->right);
            break;
        case TKN_MINUS:
            parser_expression(stream,token,node);
            node->right = create_node(NODE_ARG_LIST);
            parser_argumentListRest(stream,token,node->right);
            break;
        case TKN_OPENING_PARENTHESES:

        case TKN_INT:
            node->data_type=TYPE_INT;
            parser_expression(stream,token,node);
            node->right = create_node(NODE_ARG_LIST);
            parser_argumentListRest(stream,token,node->right);
            break;
        case TKN_DOUBLE:
            node->data_type=TYPE_DOUBLE;
            parser_expression(stream,token,node);
            node->right = create_node(NODE_ARG_LIST);
            parser_argumentListRest(stream,token,node->right);
            break;
        case TKN_UNIT:
            node->data_type=TYPE_UNIT;
            parser_expression(stream,token,node);
            node->right = create_node(NODE_ARG_LIST);
            parser_argumentListRest(stream,token,node->right);
            break;
        case TKN_STRING:
            node->data_type=TYPE_STRING;
            parser_expression(stream,token,node);
            node->right = create_node(NODE_ARG_LIST);
            parser_argumentListRest(stream,token,node->right);
            break;
        case TKN_CLOSING_PARENTHESES:
            //epsilon
            break;
        default:
        exit(2);
            //TODO ERROR
            break;
    }
}

void parser_argumentListRest(FILE* stream,token_t *token,AST_Node*node){
        *token = getToken(stream);
        if(token->type == TKN_COMMA){

            *token = getToken(stream);
            parser_expression(stream,token,node);
            node->next = create_node(NODE_ARG_LIST);
            parser_argumentListRest(stream,token,node->next);

        }else if(token->type == TKN_CLOSING_PARENTHESES){
            //epsilon
        }else{
            exit(2);           
            //TODO ERROR
        }
}

Precedence parser_get_precedence(Precedence_Symbol stack_symbol,Precedence_Symbol coming_symbol){
    switch (table[stack_symbol][coming_symbol]){
        case '>':
            return P_GREATER;
            
        case '<':
            return P_LESS;
            
        case '=':
            return P_EQUAL;
            
        default:
            exit(2);
            //ERROR
            break;
    }
} 

void parser_expression(FILE*stream,token_t *token,AST_Node *node){
    Stack *stack = stack_initialize();
    Precedence_Symbol coming_symbol = parser_TKN_to_PS(*token,stack);
    Precedence_Symbol stack_symbol = stack_top_precedence_symbol(stack);
    Precedence precedence = parser_get_precedence(stack_symbol,coming_symbol);

    stack = stack_push_precedence(stack,precedence);
    stack = stack_push(stack,STACK_TERM,coming_symbol,*token);
    
    *token = getToken(stream);
    while(token->type != TKN_NEWLINE && 
            token->type != TKN_THEN  &&
            token->type != TKN_DO &&
            token->type != TKN_SEMICOLON &&
            token->type != TKN_COLON){
        
        coming_symbol = parser_TKN_to_PS(*token,stack);
        stack_symbol = stack_top_precedence_symbol(stack);
        precedence = parser_get_precedence(stack_symbol,coming_symbol);
        if(precedence == P_GREATER){

            parser_check_exp_rules(stack);
            stack_symbol = stack_top_precedence_symbol(stack);
            precedence = parser_get_precedence(stack_symbol,coming_symbol);
            stack = stack_push_precedence(stack,precedence);
            stack =stack_push(stack,STACK_TERM,coming_symbol,*token);

        }else{

            stack = stack_push_precedence(stack,precedence);
            stack =stack_push(stack,STACK_TERM,coming_symbol,*token);
        }
        
        
        

        *token = getToken(stream);
    }

}

Stack* parser_check_exp_rules(Stack *stack){
    if (stack->category == STACK_TERM){
        token_t token = stack->token;
        stack = stack_pop(stack);
        if(stack->category == STACK_PRECEDENCE && stack->precedence == '<'){
            stack = stack_pop(stack);
            AST_Node *tree = create_node(NODE_EXP);
            tree->name=token.string;
            stack = stack_push_AST_node(stack,tree);
        }else{
            //ERROR
            exit(2);
        }
        
    }else if(stack->category ==STACK_NONTERM){
        token_t token = stack->token;
        AST_Node *tree = stack->tree;
        stack = stack_pop(stack);
        if(stack->symbol == PS_UNARY_MINUS){
            AST_Node* unary = create_node(NODE_EXP);
            unary->left = tree;
            unary->operator_type = OT_UNARY_MINUS;
            stack = stack_pop(stack);
        }else if( stack->symbol == PS_UNARY_PLUS){
            AST_Node* unary = create_node(NODE_EXP);
            unary->left = tree;
            unary->operator_type = OT_UNARY_PLUS;
            stack = stack_pop(stack);
            
            if(stack->category == STACK_PRECEDENCE && stack->precedence == '<'){
                stack = stack_pop(stack);
                stack = stack_push_AST_node(stack,unary);
            }else{
                //ERROR
                exit(2);
            }
        }else if(stack->symbol == PS_OPENING_PARENTHESES){
            
            stack = stack_pop(stack);
            if(stack->category == STACK_NONTERM){
                stack = stack_pop(stack);
                if(stack->symbol== PS_CLOSING_PARENTHESES){
                    stack = stack_pop(stack);
                    if(stack->category == STACK_PRECEDENCE && stack->precedence == P_LESS ){
                        stack = stack_pop(stack);
                        stack = stack_push_AST_node(stack,tree); //TODO
                    }else{
                        //ERROR
                        exit(2);
                    }
                }else{
                    //ERROR
                    exit(2);
                }
            }else{
                //ERROR
                exit(2);
            }

        }else if( stack->symbol !=PS_END || stack->symbol != PS_ID || stack->symbol != PS_CLOSING_PARENTHESES){
            AST_Node *exp = create_node(NODE_EXP);
            switch(stack->symbol){
                case PS_DIVISION:
                    exp->operator_type = OT_DIVIDE;
                    break;
                case PS_MINUS:
                    exp->operator_type = OT_MINUS;
                    break;
                case PS_PLUS:
                    exp->operator_type = OT_PLUS;
                    break;
                case PS_TIMES:
                    exp->operator_type = OT_TIMES;
                    break;
                case PS_LESS:
                    exp->operator_type = OT_LESS;
                    break;
                case PS_GREATER:
                    exp->operator_type = OT_GREATER;
                    break;
                case PS_EQUAL:
                    exp->operator_type = OT_EQUAL;
                    break;
                case PS_NOT_EQUAL:
                    exp->operator_type = OT_NOT_EQUAL;
                    break;
                case PS_LESS_EQUAL:
                    exp->operator_type = OT_LESS_EQUAL;
                    break;
                case PS_GREATER_EQUAL:
                    exp->operator_type = OT_GREATER_EQUAL;
                    break;
                default:
                //ERROR
                exit(2);
            }
            exp->left=tree;
            stack = stack_pop(stack);
            if(stack->category == STACK_NONTERM){
                exp->right = create_node(NODE_EXP);
                stack = stack_pop(stack);
                if(stack->category == STACK_PRECEDENCE && stack->precedence == '<'){
                    stack = stack_pop(stack);
                    stack = stack_push_AST_node(stack,exp);
                }else{
                    //ERROR
                    exit(2);
                }
            }else{
                //ERROR
                exit(2);
            }
        }
    }else{
        exit(2);
        //ERROR
    }
} 

Precedence_Symbol parser_TKN_to_PS(token_t token,Stack*stack){
    switch(token.type){
        case TKN_PLUS:
            if (stack_top_precedence_symbol(stack) == PS_CLOSING_PARENTHESES || stack_top_precedence_symbol(stack) == PS_ID ){
                return PS_PLUS;
            }else{
                return PS_UNARY_PLUS;
            }
        case TKN_MINUS:
            if (stack_top_precedence_symbol(stack) == PS_CLOSING_PARENTHESES || stack_top_precedence_symbol(stack) == PS_ID ){
                return PS_MINUS;
            }else{
                return PS_UNARY_MINUS;
            }
        case TKN_DIVISION_SLASH:
            return PS_DIVISION;
            
        case TKN_TIMES:
            return PS_TIMES;
            
        case TKN_NOT_EQUAL:
            return PS_NOT_EQUAL;
            
        case TKN_EQUAL:
            return PS_EQUAL;
            
        case TKN_LESS:
            return PS_LESS;
            
        case TKN_LESS_EQUAL:
            return PS_LESS_EQUAL;
            
        case TKN_GREATER:
            return PS_GREATER;
            
        case TKN_GREATER_EQUAL:
            return PS_GREATER_EQUAL;
            
        case TKN_OPENING_PARENTHESES:
            return PS_OPENING_PARENTHESES;
            
        case TKN_CLOSING_PARENTHESES:
            return PS_CLOSING_PARENTHESES;
            
        case TKN_ID:
        case TKN_INT:
        case TKN_UNIT:
        case TKN_DOUBLE:
        case TKN_STRING:
            return PS_ID;
            
        default:
        //ERROR
        exit(2);
    }
}

//TODO FILL HTAB
//TODO ERRORfprintf