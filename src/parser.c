//autor: xmiklej00
#include <stdio.h>
#include <scanner.h>
#include <stdbool.h>
#include <parser.h>

void parser(FILE* stream);

void parser(FILE* stream){

    token_t token = getToken(stream);

    if(token.type == TKN_IMPORT){
        parser_prolog(stream);
        token= getToken(stream);

        if(token.type == TKN_DEF || token.type == TKN_AT_MAIN){
            parser_definitionList(stream,token);
        }
    }
    else {
        //TODO ERROR
    }
}

void parser_prolog(FILE* stream ){
    token_t token = getToken(stream);

    if(token.type == TKN_IFJ26dot_asterisk){
        
        token = getToken(stream);
        if (token.type == TKN_NEWLINE){

        }
    }else{
        //TODO ERROR
    }

}

void parser_definitionList(FILE* stream ,token_t token){
    while(token.type != TKN_EOF){
        if(token.type == TKN_DEF){

            parser_defFunction(stream);

        }else if(token.type == TKN_AT_MAIN){
            
            parser_defMainFunc(stream);
    
        }else{//TODO ERROR
        break;
        }
        token= getToken(stream);    
    }

        
}

void parser_defFunction(FILE*stream){

    token_t token = getToken(stream);

    if(token.type == TKN_ID){
        token = getToken(stream);

        if(token.type == TKN_OPENING_PARENTHESES){

            parser_parametList(stream);
            token = getToken(stream);
            
            if (token.type == TKN_CLOSING_PARENTHESES){
                token = getToken(stream);
            
                if(token.type ==TKN_COLON ){
                    parser_type(stream);
                    token = getToken(stream);
                    if (token.type == TKN_ASSING){
                        parser_body(stream);
                    }
                }
                else if(token.type == TKN_ASSING){
                    parser_body(stream);

                }             
            }
        }
    }else{
        //TODO ERROR
    }
    

}

void parser_defMainFunc(FILE* stream){
    token_t token = getToken(stream);

    if (token.type == TKN_NEWLINE){
        token = getToken(stream);
        
        if(token.type == TKN_DEF){
        token = getToken(stream);

            if(token.type == TKN_ID){
                token=getToken(stream);

                if(token.type == TKN_OPENING_PARENTHESES){
                    parser_parametList(stream);
                    token = getToken(stream);

                    if(token.type == TKN_CLOSING_PARENTHESES){
                        token = getToken(stream);

                        if(token.type == TKN_COLON){
                            parser_type(stream);
                            
                            token = getToken(stream);

                            if(token.type == TKN_ASSING){
                                parse_body(stream);
                            }

                        }
                        
                    }

                }
            }

        }
    }
    else if(token.type == TKN_DEF){
        token = getToken(stream);

        if(token.type == TKN_ID){
            token=getToken(stream);

            if(token.type == TKN_OPENING_PARENTHESES){
                parser_parametList(stream);
                token = getToken(stream);

                if(token.type == TKN_CLOSING_PARENTHESES){
                    token = getToken(stream);

                    if(token.type == TKN_COLON){
                        parser_type(stream);
                            
                        token = getToken(stream);

                        if(token.type == TKN_ASSING){
                            parse_body(stream);
                        }

                    }
                        
                }

            }
        }
    }
    else{
        //TODO ERROR
    }

}

void parser_type(FILE*stream){
    token_t token = getToken(stream);

    switch (token.type){
        case TKN_INT_KEYWORD:
        case TKN_DOUBLE_KEYWORD:
        case TKN_UNIT:
        case TKN_STRING_KEYWORD:
        break;
        default:
        //TODO ERROR
        break;
    }
        
    

}

void parser_body(FILE*stream){
    token_t token = getToken(stream);

    if(token.type == TKN_NEWLINE){
        parser_block(stream);

    }
    else if(token.type == TKN_IF){
        parser_ifStatement(stream);

    }
    else if(token.type == TKN_ID||token.type == TKN_MINUS || token.type == TKN_OPENING_PARENTHESES || token.type == TKN_INT||token.type == TKN_DOUBLE|| token.type == TKN_UNIT || token.type == TKN_STRING){ 
        parser_expretion(stream,token);
        token = getToken(stream);
        if(token.type != TKN_NEWLINE){
            //TODO ERROR
        }

    }
}

void parser_block(FILE* stream){
    token_t token = getToken(stream);

    if (token.type == TKN_INDENTATION){
        parser_statementList(stream);
        token = getToken(stream);
        if(token.type!=TKN_DEDENTATION){
            //TODO ERROR
        }
    }else{
        //TODO ERROR
    }
}
    

void parser_statementList(FILE* stream){
    token_t token = getToken(stream);

    while(token.type != TKN_DEDENTATION && token.type != TKN_EOF){
        switch (token.type){
            case TKN_VAL:
                parser_valDef(stream);
                parser_between(stream,token);
            break;
            case TKN_VAR:
                parser_varDef(stream);
                parser_between(stream,token);
            break;
            case TKN_IF:
                parser_ifStatement(stream);
            break;
            case TKN_WHILE:
                parser_whileLoop(stream);
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
                parser_returnRest(stream);
                parser_between(stream,token);
            break;
            default:
                //TODO ERROR
            break;
        } 
        token = getToken(stream);
    }
}

void parser_between(FILE* stream,token_t token){
    switch (token.type){

        case TKN_NEWLINE:
        break;
        case TKN_SEMICOLON:
        break;
        case TKN_DEDENTATION:
        case TKN_EOF:
        break;
        default:
        //TODO ERROR
        break;
    }
}

void parser_valDef(FILE*stream){
    token_t token = getToken(stream);
    if (token.type == TKN_ID){
        parser_optType(stream,token);
        token = getToken(stream);
        if(token.type == TKN_ASSING){
            parser_expretion(stream,token);
        }
    }else{
    //TODO ERROR
    }
}

void parser_optType(FILE* stream,token_t token){
    if(token.type == TKN_COLON){
       parser_type(stream);
       
    }
}

void parser_varDef (FILE* stream){
    token_t token = getToken(stream);
    if (token.type == TKN_ID){
        parser_optType(stream,token);
        token = getToken(stream);
        if(token.type == TKN_ASSING){
            parser_expretion(stream,token);
        }
    }else{
        //TODO ERROR
    }
}

void parser_ifStatement (FILE* stream){
    token_t token = getToken(stream);
    parser_expretion(stream,token);
    if (token.type == TKN_THEN){
        parser_body(stream);
        parser_elsePart(stream);
    }else{
        //TODO ERROR
    }
}

void parser_elsePart(FILE* stream){
    token_t token= getToken(stream);
    switch(token.type){
        case TKN_ELSE:
            parser_body(stream);
        break;
        case TKN_NEWLINE:
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
        break;
        default:
        //TODO ERROR
        break;
    }

}

void parser_whileLoop(FILE* stream){
    token_t token = getToken(stream);
    parser_expretion(stream,token);
    if(token.type == TKN_DO){
        parser_body(stream);
    }else{
        //TODO ERROR
    }

}

void parser_returnRest(FILE* stream){
    token_t token = getToken(stream);
    switch (token.type){
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
        //TODO ERROR
        break;
    }
}

void parser_parametList(FILE*stream){
    token_t token =getToken(stream);
    
    if(token.type == TKN_ID){
        parser_paramet(stream);
        parser_parametListRest(stream);
    }
    else if(token.type == TKN_CLOSING_PARENTHESES){

    }else{
        //TODO ERROR
    }
}

void parser_paramet(FILE*stream){
    token_t token =getToken(stream);
    if(token.type == TKN_COLON){
        parser_type(stream);
    }else{
        //TODO ERROR
    }
}

void parser_parametListRest(FILE*stream){
    token_t token = getToken(stream);

    while(token.type!=TKN_CLOSING_PARENTHESES){

        if(token.type == TKN_COMMA){
            token_t token = getToken(stream);
            if(token.type==TKN_ID){
                parser_paramet(stream);
            }
        }else{
            //TODO ERROR
            break;
        }
        token = getToken(stream);
    }
}

void parser_statementRest(FILE*stream,token_t token){
    if(token.type == TKN_ID){
        token = getToken(stream);
        switch(token.type){
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
                parser_argumentList(stream);
                break;
            case TKN_NEWLINE:
            case TKN_DEDENTATION:
            case TKN_CLOSING_PARENTHESES:
            case TKN_SEMICOLON:
            case TKN_EOF:
                break;
            default:
            //TODO ERROR
            break;

        }
    }
}

void parser_argumentList(FILE*stream){
    token_t token = getToken(stream);

    switch (token.type){
        case TKN_ID:
        case TKN_MINUS:
        case TKN_OPENING_PARENTHESES:
        case TKN_INT:
        case TKN_DOUBLE:
        case TKN_UNIT:
        case TKN_STRING:
            parser_expretion(stream,token);
            parser_argumentListRest(stream);
            break;
        case TKN_CLOSING_PARENTHESES:
            break;
        default:
            //TODO ERROR
            break;
    }
}

void parser_argumentListRest(FILE* stream){
        token_t token = getToken(stream);
        if(token.type == TKN_COMMA){
            token = getToken(stream);
            parser_expretion(stream,token);
            parser_argumentListRest(stream);
        }else if(token.type == TKN_CLOSING_PARENTHESES){

        }else{
            //TODO ERROR
        }
}