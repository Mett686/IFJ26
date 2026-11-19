//autor: xmiklej00
#include <stdio.h>
#include <scanner.h>
#include <stdbool.h>
#include <parser.h>
#include <stdlib.h>

void parser(FILE* stream);

AST_Node* AST_inicial(){
    AST_Node *table = mallock(sizeof(AST_Node));
    if(table==NULL){
        free(table);
        exit(-1);
    }
    table->left =NULL;
    table->right = NULL;
    table->next = NULL;
    table->node_type = NODE_PROGRAM;

    return table;
}

void create_node(Node_type type,AST_Node *node){
    AST_Node *new_node = mallock(sizeof(AST_Node));
    if(new_node==NULL){
        return 1; 
    }
    node=new_node;
    node->node_type=type;
    node->left=NULL;
    node->right=NULL;
    node->next=NULL;
    
}

void parser(FILE* stream){
    AST_Node *tree = AST_inicial();
    token_t token = getToken(stream);

    if(token.type == TKN_IMPORT){
        parser_prolog(stream,&token);
        token= getToken(stream);

        if(token.type == TKN_DEF || token.type == TKN_AT_MAIN){
            parser_definitionList(stream,&token,tree);
        }
    }
    else {
        //TODO ERROR
        exit(2);
    }
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

void parser_definitionList(FILE* stream ,token_t *token,AST_Node *tree){
    AST_Node *head = tree;
    while(token->type != TKN_EOF){
        

        if(token->type == TKN_DEF){

            parser_defFunction(stream,token,tree->left);

        }else if(token->type == TKN_AT_MAIN){
            
            parser_defFunction(stream,token,tree->left);

            parser_defMainFunc(stream,token,tree->left);

            
    
        }else{//TODO ERROR
            exit(2);
        }

        create_node(NODE_DEF_LIST,tree->right);
        tree=tree->right;
        *token= getToken(stream);
            
    }

        
}

void parser_defFunction(FILE*stream,token_t *token,AST_Node*node){

     *token = getToken(stream);

    if(token->type == TKN_ID){
        create_node(NODE_FUNC_DEF,node);
        node->name = token->string;
        *token = getToken(stream);

        if(token->type == TKN_OPENING_PARENTHESES){
            create_node(NODE_PARAM,node->left);
            parser_parametList(stream,token,node->left);
            
            
            if (token->type == TKN_CLOSING_PARENTHESES){
                *token = getToken(stream);
            
                if(token->type ==TKN_COLON ){
                    
                    parser_type(stream,token,node);
                    *token = getToken(stream);

                    if (token->type == TKN_ASSING){
                        parser_body(stream,token,node->right);
                    }
                }
                else if(token->type == TKN_ASSING){
                    
                    node->data_type=TYPE_NULL;
                    parser_body(stream,token,node->right);
                
                }else{
                    //ERROR
                }             
            }else{
                //ERROR
            }
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
                create_node(NODE_FUNC_DEF_MAIN,node);
                node->name=token->string;
                *token=getToken(stream);

                if(token->type == TKN_OPENING_PARENTHESES){
                    create_node(NODE_PARAM,node->left);
                    parser_parametList(stream,token,node->left);
                    

                    if(token->type == TKN_CLOSING_PARENTHESES){
                        *token = getToken(stream);

                        if(token->type == TKN_COLON){
                            parser_type(stream,token,node);
                            
                            *token = getToken(stream);

                            if(token->type == TKN_ASSING){

                                parser_body(stream,token,node->right);
                            }

                        }else if (token->type == TKN_ASSING){
                            node->data_type=TYPE_NULL;
                            parse_body(stream,token);
                        }else{
                            //ERROR
                        }
                        
                    }else{
                        //ERROR
                    }

                }else{
                    //ERROR
                }
            }else{
                //ERROR
            }

        }else{
            //ERROR
        }
    }
    else if(token->type == TKN_DEF){
        *token = getToken(stream);



        if(token->type == TKN_ID){
            create_node(NODE_FUNC_DEF_MAIN,node);
            node->name = token->string;
            *token=getToken(stream);

            if(token->type == TKN_OPENING_PARENTHESES){
                create_node(NODE_PARAM,node->left);
                parser_parametList(stream,token,node->left);
                

                if(token->type == TKN_CLOSING_PARENTHESES){
                    *token = getToken(stream);

                    if(token->type == TKN_COLON){
                        parser_type(stream,token,node);
                            
                        *token = getToken(stream);

                        if(token->type == TKN_ASSING){
                            parse_body(stream,token,node->right);
                        }

                    }else if(token->type == TKN_ASSING){
                        node->data_type=TYPE_NULL;
                        parser_body(stream,token,node->right);
                    }else{
                        //ERROR
                    }
                        
                }else{
                    //ERROR
                }

            }else{
                //ERROR
            }
        }else{
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
        parser_expretion(stream,token,node);
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
        parser_statementList(stream,token,node);
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
                create_node(NODE_VAL_DEF,node->left);
                parser_valDef(stream,token,node->left);
                parser_between(stream,token);
            break;
            case TKN_VAR:
                create_node(NODE_VAR_DEF,node->left);
                parser_varDef(stream,token,node->left);
                parser_between(stream,token);
            break;
            case TKN_IF:
                create_node(NODE_IF,node->left);
                parser_ifStatement(stream,token,node->left);
            break;
            case TKN_WHILE:
                create_node(NODE_WHILE_LOOP,node->left);
                parser_whileLoop(stream,token,node->left);
            break;
            case TKN_ID:
                create_node(NODE_STATEMENT,node->left);
                node->name=token->string;
                parser_statementRest(stream,token,node->left);
                parser_between(stream,token);
            break;
            case TKN_INT:
                create_node(NODE_EXP,node->left);
                node->data_type=TYPE_INT;
                parser_expretion(stream,token,node->left);
                parser_between(stream,token);
                break;
            case TKN_DOUBLE:
                create_node(NODE_EXP,node->left);
                node->data_type=TYPE_DOUBLE;
                parser_expretion(stream,token,node->left);
                parser_between(stream,token);
                break;
            case TKN_UNIT:
                create_node(NODE_EXP,node->left);
                node->data_type=TYPE_UNIT;
                parser_expretion(stream,token,node->left);
                parser_between(stream,token);
                break;
            case TKN_STRING:
                create_node(NODE_EXP,node->left);
                node->data_type=TYPE_STRING;
                parser_expretion(stream,token,node->left);
                parser_between(stream,token);
            break;
            case TKN_OPENING_PARENTHESES:
                create_node(NODE_EXP,node->left);
                parser_expretion(stream,token,node->left);
                parser_between(stream,token);
            break;
            case TKN_RETURN:
                create_node(NODE_RETURN,node->left);
                parser_returnRest(stream,token,node->left);
                parser_between(stream,token);
            break;
            default:
            exit(2);
                //TODO ERROR
            break;
        } 
        create_node(NODE_STATEMENT_LIST,node->right);
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
            
            create_node(NODE_EXP,node->right);
            parser_expretion(stream,token,node->right);

        }else{
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

            create_node(NODE_EXP,node->right);
            parser_expretion(stream,token,node->right);
        
        }else{
            //ERROR
        }
    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_ifStatement (FILE* stream,token_t *token,AST_Node *node){
    *token = getToken(stream);
    create_node(NODE_EXP,node->left);
    parser_expretion(stream,token,node->left);
    
    if (token->type == TKN_THEN){
        parser_body(stream,token,node->right);
        create_node(NODE_ELSE,node->next);
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
            parser_body(stream,token,node);
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
    create_node(NODE_WHILE_LOOP,node);
    create_node(NODE_EXP,node->left);
    parser_expretion(stream,token,node->left);
    
    if(token->type == TKN_DO){
        create_node(NODE_BLOCK,node->right);
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
            create_node(NODE_EXP,node->left);
            node->left->name = token->string;
            parser_expretion(stream,token,node->left);
            break;
        case TKN_MINUS:
            reate_node(NODE_EXP,node->left);
            parser_expretion(stream,token,node->left);
            break;
        case TKN_INT:
            create_node(NODE_EXP,node->left);
            node->left->data_type = TYPE_UNIT;
            parser_expretion(stream,token,node->left);
            break;
        case TKN_DOUBLE:create_node(NODE_EXP,node->left);
            node->left->data_type = TYPE_UNIT;
            parser_expretion(stream,token,node->left);
            break;
        case TKN_STRING:
            create_node(NODE_EXP,node->left);
            node->left->data_type = TYPE_UNIT;
            parser_expretion(stream,token,node->left);
            break;
        case TKN_UNIT:
            create_node(NODE_EXP,node->left);
            node->left->data_type = TYPE_UNIT;
            parser_expretion(stream,token,node->left);
            break;
        case TKN_OPENING_PARENTHESES:
            create_node(NODE_EXP,node->left);
            parser_expretion(stream,token,node->left);
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

void parser_parametList(FILE*stream,token_t *token,AST_Node*node){
    
    *token = getToken(stream);
    
    if(token->type == TKN_ID){
        node->name= token->string;
        
        parser_paramet(stream,token,node);
        parser_parametListRest(stream,token,node->next);
    }
    else if(token->type == TKN_CLOSING_PARENTHESES){

    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_paramet(FILE*stream,token_t *token,AST_Node *node){
    //TODO ULOZENIE ID DO TABULKY
    *token =getToken(stream);
    if(token->type == TKN_COLON){
        parser_type(stream,token,node);
    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_parametListRest(FILE*stream,token_t *token,AST_Node *node){
    *token = getToken(stream);

    while(token->type!=TKN_CLOSING_PARENTHESES){

        if(token->type == TKN_COMMA){
            *token = getToken(stream);
            if(token->type==TKN_ID){
                create_node(NODE_PARAM,node);
                node->name=token->string;
                parser_paramet(stream,token,node);
            }else{
                //TODO ERROR
            }
        }else{
            exit(2);
            //TODO ERROR
            break;
        }

        node=node->next;

        *token = getToken(stream);
    }
}

void parser_statementRest(FILE*stream,token_t *token,AST_Node*node){
    if(token->type == TKN_ID){
        *token = getToken(stream);
        switch(token->type){
            case TKN_MINUS:
                create_node(NODE_EXP,node);
                parser_expretion(stream,token,node);
                break;
            case TKN_DIVISION_SLASH:
                create_node(NODE_EXP,node);
                parser_expretion(stream,token,node);
                break;
            case TKN_TIMES:
                create_node(NODE_EXP,node);
                parser_expretion(stream,token,node);
                break;
            case TKN_NOT_EQUAL:
                create_node(NODE_EXP,node);
                parser_expretion(stream,token,node);
                break;
            case TKN_EQUAL:
                create_node(NODE_EXP,node);
                parser_expretion(stream,token,node);
                break;
            case TKN_LESS:
                create_node(NODE_EXP,node);
                parser_expretion(stream,token,node);
                break;
            case TKN_LESS_EQUAL:
                create_node(NODE_EXP,node);
                parser_expretion(stream,token,node);
                break;
            case TKN_GREATER:
                create_node(NODE_EXP,node);
                parser_expretion(stream,token,node);
                break;
            case TKN_GREATER_EQUAL:
                create_node(NODE_EXP,node);
                parser_expretion(stream,token,node);
                break;
            case TKN_PLUS:
                create_node(NODE_EXP,node);
                parser_expretion(stream,token,node);
                break;
            case TKN_OPENING_PARENTHESES:
                create_node(NODE_ARG_LIST,node);
                parser_argumentList(stream,token,node);
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
            parser_expretion(stream,token,node);
            create_node(NODE_ARG_LIST,node->next);
            parser_argumentListRest(stream,token,node->next);
            break;
        case TKN_MINUS:
            parser_expretion(stream,token,node);
            create_node(NODE_ARG_LIST,node->next);
            parser_argumentListRest(stream,token,node->next);
            break;
        case TKN_OPENING_PARENTHESES:

        case TKN_INT:
            node->data_type=TYPE_INT;
            parser_expretion(stream,token,node);
            create_node(NODE_ARG_LIST,node->next);
            parser_argumentListRest(stream,token,node->next);
            break;
        case TKN_DOUBLE:
            node->data_type=TYPE_DOUBLE;
            parser_expretion(stream,token,node);
            create_node(NODE_ARG_LIST,node->next);
            parser_argumentListRest(stream,token,node->next);
            break;
        case TKN_UNIT:
            node->data_type=TYPE_UNIT;
            parser_expretion(stream,token,node);
            create_node(NODE_ARG_LIST,node->next);
            parser_argumentListRest(stream,token,node->next);
            break;
        case TKN_STRING:
            node->data_type=TYPE_STRING;
            parser_expretion(stream,token,node);
            create_node(NODE_ARG_LIST,node->next);
            parser_argumentListRest(stream,token,node->next);
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
            parser_expretion(stream,token,node);
            create_node(NODE_ARG_LIST,node->next);
            parser_argumentListRest(stream,token,node->next);
        }else if(token->type == TKN_CLOSING_PARENTHESES){
            //epsilon
        }else{
            exit(2);           
            //TODO ERROR
        }
}

Stack * stack_iniciaize(){
    Stack *new_stack=calloc(1,sizeof(Stack));
    if(new_stack!=NULL){
        new_stack->next=NULL;
        new_stack->category=STACK_TERM;
        new_stack->symbol=PS_END;
        
        return new_stack;
    }else{
        //ERROR
        return NULL;
    }
}

Stack* stack_push(Stack *stack,Symbol_Category category,Precedens_Symbol symbol, token_t *token){
    Stack *new_stack = malloc(sizeof(Stack));
    if(new_stack==NULL){
        //ERROR
        return NULL;
    }else{
        new_stack->next = stack;
        new_stack->category=category;
        new_stack->symbol=symbol;
        new_stack->token = *token;
        return new_stack;
    }
}

Stack* stack_pop(Stack *stack){
    Stack *tmp= stack->next;
    free(stack);
    
    return tmp;
}