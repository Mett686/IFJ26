//autor: xmiklej00
#include <stdio.h>
#include <scanner.h>
#include <stdbool.h>
#include <parser.h>

vooid parser();

vooid parser(FILE* stream){

    token_t token = getToken(stream);

    if(token->type == TKN_IMPORT){
        prolog(stream);
        token= getToken(stream);

        if(token->type == TKN_DEF || token->type == TKN_AT_MAIN){
            parser_definitionList(stream,token);
        }
    }
    else 
}

void parser_prolog(FILE* stream ){
    token_t token = getToken(stream);

    if(token->type != TKN_IFJ26dot_asterisk){
        //TODO ERROR
    }

}

void parser_definitionList(FILE* stream token_t token){
    while(token != TKN_EOF){
        if(token->type == TKN_DEF){

            parser_defFunction(stream);

        }else if(token->type == TKN_AT_MAIN){
            
            parser_defMainFunc(stream);
    
        }else
        //TODO ERROR
        }

        token = getToken;
}

void parser_defFunction(FILE*stream){

    token_t token = getToken(stream);

    if(token->type == TKN_ID){
        token = getToken(stream);

        if(token->type == TKN_OPENING_PARENTHESES){

            parser_parametList(stream);
            token = getToken(stream);
            
            if (token->type == TKN_CLOSING_PARENTHESES){
                token = getToken(stream)\;
            
                if(token->type ==TKN_COLON ){
                    parser_type(stream);
                
                }
                else if(token->type == TKN_ASSING){
                    parse_body(stream);

                }             
            }
        }
    }else{
        //TODO ERROR
    }
    

}

void parser_defMainFunc(FILE* stream){
    token_t token = getToken(stream);

    if (token->type == TKN_NEWLINE){
        token = getToken(stream);

        if(token->type == TKN_DEF){
            token = getToken(stream);

            if(token->type == TKN_ID){
                token=getToken(stream);

                if(token->type == TKN_OPENING_PARENTHESES){
                    parser_parametList(stream);
                    token = getToken(stream);

                    if(token->type == TKN_CLOSING_PARENTHESES){
                        token = getToken(stream);

                        if(token->type == TKN_COLON){
                            parser_type(stream);
                            
                            token = getToken(stream);

                            if(token->type == TKN_ASSING){
                                parse_body(stream);
                            }

                        }
                        
                    }

                }
            }

        }
    }else{
        //TODO ERROR
    }

}

void parser_type(FILE*stream){
    token_t token = getToken

    if (token->type == TKN_INT){

    }else if (token->type == TKN_DOUBLE){

    }else if (token->type == TKN_UNIT){

    }else if (token->type == TKN_BOOLEAN){
        
    }else{
        //TODO ERROR
    }

}

void parser_body(FILE*stream){
    
}