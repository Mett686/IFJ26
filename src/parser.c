//autor: xmiklej00
#include <stdio.h>
#include <scanner.h>
#include <stdbool.h>
#include <parser.h>

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
    table->type = NODE_PROGRAM;
    return table;
}

AST_Node* create_node(Node_type type,AST_Node *node){
    AST_Node *new_node = mallock(sizeof(AST_Node));
    if(new_node==NULL){
        return 1; 
    }
    node=new_node;
    node->type=type;
    node->left=NULL;
    node->right=NULL;
    node->next=NULL;
}

void parser(FILE* stream){

    token_t token = getToken(stream);

    if(token.type == TKN_IMPORT){
        parser_prolog(stream,&token);
        token= getToken(stream);

        if(token.type == TKN_DEF || token.type == TKN_AT_MAIN){
            parser_definitionList(stream,&token);
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

void parser_definitionList(FILE* stream ,token_t *token){
    while(token->type != TKN_EOF){
        if(token->type == TKN_DEF){

            parser_defFunction(stream,token);

        }else if(token->type == TKN_AT_MAIN){
            
            parser_defMainFunc(stream,token);
    
        }else{//TODO ERROR
            exit(2);
        break;
        }
        *token= getToken(stream);    
    }

        
}

void parser_defFunction(FILE*stream,token_t *token){

     *token = getToken(stream);

    if(token->type == TKN_ID){
        *token = getToken(stream);

        if(token->type == TKN_OPENING_PARENTHESES){

            parser_parametList(stream,token);
            
            
            if (token->type == TKN_CLOSING_PARENTHESES){
                *token = getToken(stream);
            
                if(token->type ==TKN_COLON ){
                    parser_type(stream,token);
                    *token = getToken(stream);
                    if (token->type == TKN_ASSING){
                        parser_body(stream,token);
                    }
                }
                else if(token->type == TKN_ASSING){
                    parser_body(stream,token);

                }             
            }
        }
    }else{
        exit(2);
        //TODO ERROR
    }
    

}

void parser_defMainFunc(FILE* stream,token_t *token){
    *token = getToken(stream);

    if (token->type == TKN_NEWLINE){
        *token = getToken(stream);
        
        if(token->type == TKN_DEF){
        *token = getToken(stream);

            if(token->type == TKN_ID){
                *token=getToken(stream);

                if(token->type == TKN_OPENING_PARENTHESES){
                    parser_parametList(stream,token);
                    

                    if(token->type == TKN_CLOSING_PARENTHESES){
                        *token = getToken(stream);

                        if(token->type == TKN_COLON){
                            parser_type(stream,token);
                            
                            *token = getToken(stream);

                            if(token->type == TKN_ASSING){
                                parse_body(stream,token);
                            }

                        }else if (token->type == TKN_ASSING){
                            parse_body(stream,token);
                        }else{
                            //ERROR
                        }
                        
                    }

                }
            }

        }
    }
    else if(token->type == TKN_DEF){
        *token = getToken(stream);



        if(token->type == TKN_ID){
            *token=getToken(stream);

            if(token->type == TKN_OPENING_PARENTHESES){
                parser_parametList(stream,token);
                

                if(token->type == TKN_CLOSING_PARENTHESES){
                    *token = getToken(stream);

                    if(token->type == TKN_COLON){
                        parser_type(stream,token);
                            
                        *token = getToken(stream);

                        if(token->type == TKN_ASSING){
                            parse_body(stream,token);
                        }

                    }else if(token->type == TKN_ASSING){
                        parser_body(stream,token);
                    }
                        
                }

            }
        }
    }
    else{
        exit(2);
        //TODO ERROR
    }

}

void parser_type(FILE*stream,token_t *token){
    *token = getToken(stream);

    switch (token->type){
        case TKN_INT_KEYWORD:
        case TKN_DOUBLE_KEYWORD:
        case TKN_UNIT_KEYWORD:
        case TKN_STRING_KEYWORD:
        break;
        default:
        exit(2);
        //TODO ERROR
        break;
    }
        
    

}

void parser_body(FILE*stream,token_t *token){
    *token = getToken(stream);

    switch (token->type)
    {
    case TKN_NEWLINE:
        parser_block(stream,token);
        break;
    case TKN_ID:
        parser_statementRest(stream,token);
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
        parser_expretion(stream,token);
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

void parser_block(FILE* stream,token_t *token){
    *token = getToken(stream);

    if (token->type == TKN_INDENTATION){
        parser_statementList(stream,token);
        *token = getToken(stream);
        if(token->type!=TKN_DEDENTATION){
            exit(2);//TODO ERROR
        }
    }else{
        exit(2);
        //TODO ERROR
    }
}
    

void parser_statementList(FILE* stream,token_t *token){
    *token = getToken(stream);

    while(token->type != TKN_DEDENTATION && token->type != TKN_EOF){
        switch (token->type){
            case TKN_VAL:
                parser_valDef(stream,token);
                parser_between(stream,token);
            break;
            case TKN_VAR:
                parser_varDef(stream,token);
                parser_between(stream,token);
            break;
            case TKN_IF:
                parser_ifStatement(stream,token);
            break;
            case TKN_WHILE:
                parser_whileLoop(stream,token);
            break;
            case TKN_ID:
                parser_statementRest(stream,token);
                parser_between(stream,token);
            break;
            case TKN_COLON:
                parser_expretion(stream,token);
                parser_between(stream,token);
            break;
            case TKN_INT:
            case TKN_DOUBLE:
            case TKN_UNIT:
            case TKN_STRING:
                parser_expretion(stream,token);
                parser_between(stream,token);
            break;
            case TKN_OPENING_PARENTHESES:
                parser_expretion(stream,token);
            break;
            case TKN_RETURN:
                parser_returnRest(stream,token);
                parser_between(stream,token);
            break;
            default:
            exit(2);
                //TODO ERROR
            break;
        } 
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

void parser_valDef(FILE*stream,token_t *token){
    *token = getToken(stream);
    if (token->type == TKN_ID){
        parser_optType(stream,token);
        *token = getToken(stream);
        if(token->type == TKN_ASSING){
            parser_expretion(stream,token);
        }
    }else{
        exit(2);
    //TODO ERROR
    }
}

void parser_optType(FILE* stream,token_t *token){
    if(token->type == TKN_COLON){
       parser_type(stream,token);
       
    }
}

void parser_varDef (FILE* stream,token_t *token){
    *token = getToken(stream);
    if (token->type == TKN_ID){
        parser_optType(stream,token);
        *token = getToken(stream);
        if(token->type == TKN_ASSING){
            parser_expretion(stream,token);
        }
    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_ifStatement (FILE* stream,token_t *token){
    *token = getToken(stream);
    
    parser_expretion(stream,token);
    
    if (token->type == TKN_THEN){
        parser_body(stream,token);
        parser_elsePart(stream,token);
    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_elsePart(FILE* stream,token_t *token){
    *token= getToken(stream);
    switch(token->type){
        case TKN_ELSE:
            parser_body(stream,token);
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

void parser_whileLoop(FILE* stream,token_t *token){
    *token = getToken(stream);
    parser_expretion(stream,token);
    if(token->type == TKN_DO){
        parser_body(stream,token);
    }else{
        exit(2);
        //TODO ERROR
    }

}

void parser_returnRest(FILE* stream,token_t *token){
    *token = getToken(stream);
    switch (token->type){
        case TKN_ID:
        parser_expretion(stream,token);
        break;
        case TKN_MINUS:
        parser_expretion(stream,token);
        break;
        case TKN_INT:
        case TKN_DOUBLE:
        case TKN_STRING:
        case TKN_UNIT:
        parser_expretion(stream,token);
        break;
        case TKN_OPENING_PARENTHESES:
        parser_expretion(stream,token);
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

void parser_parametList(FILE*stream,token_t *token){
    
    *token =getToken(stream);
    
    if(token->type == TKN_ID){
        parser_paramet(stream,token);
        parser_parametListRest(stream,token);
    }
    else if(token->type == TKN_CLOSING_PARENTHESES){

    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_paramet(FILE*stream,token_t *token){
    //TODO ULOZENIE ID DO TABULKY
    *token =getToken(stream);
    if(token->type == TKN_COLON){
        parser_type(stream,token);
    }else{
        exit(2);
        //TODO ERROR
    }
}

void parser_parametListRest(FILE*stream,token_t *token){
    *token = getToken(stream);

    while(token->type!=TKN_CLOSING_PARENTHESES){

        if(token->type == TKN_COMMA){
            *token = getToken(stream);
            if(token->type==TKN_ID){
                parser_paramet(stream,token);
            }else{
                //TODO ERROR
            }
        }else{
            exit(2);
            //TODO ERROR
            break;
        }
        *token = getToken(stream);
    }
}

void parser_statementRest(FILE*stream,token_t *token){
    if(token->type == TKN_ID){
        *token = getToken(stream);
        switch(token->type){
            case TKN_MINUS:
            case TKN_DIVISION_SLASH:
            case TKN_TIMES:
            case TKN_NOT_EQUAL:
            case TKN_EQUAL:
            case TKN_LESS:
            case TKN_LESS_EQUAL:
            case TKN_GREATER:
            case TKN_GREATER_EQUAL:
            case TKN_PLUS:
                parser_expretion(stream,token);
                break;
            case TKN_OPENING_PARENTHESES:
                parser_argumentList(stream,token);
                break;
            case TKN_NEWLINE:
            case TKN_DEDENTATION:
            case TKN_CLOSING_PARENTHESES:
            case TKN_SEMICOLON:
            case TKN_EOF:
                break;
            default:
            exit(2);
            //TODO ERROR
            break;

        }
    }
}

void parser_argumentList(FILE*stream,token_t *token){
    *token = getToken(stream);

    switch (token->type){
        case TKN_ID:
        case TKN_MINUS:
        case TKN_OPENING_PARENTHESES:
        case TKN_INT:
        case TKN_DOUBLE:
        case TKN_UNIT:
        case TKN_STRING:
            parser_expretion(stream,token);
            parser_argumentListRest(stream,token);
            break;
        case TKN_CLOSING_PARENTHESES:
            break;
        default:
        exit(2);
            //TODO ERROR
            break;
    }
}

void parser_argumentListRest(FILE* stream,token_t *token){
        *token = getToken(stream);
        if(token->type == TKN_COMMA){
            *token = getToken(stream);
            parser_expretion(stream,token);
            parser_argumentListRest(stream,token);
        }else if(token->type == TKN_CLOSING_PARENTHESES){

        }else{
            exit(2);           
            //TODO ERROR
        }
}