//autor: xmiklej00

#include <stack.h>

void StackInicial (ParserStack * stack){
    stack->top=NULL;
}

bool StackPush (ParserStack* stack, NonTerminal nontoken,EtokenType token){
    StackSymbol *newSymbol = malloc(sizeof(StackSymbol));
    if(newSymbol==NULL){
        return false;
    }
    newSymbol->next=stack->top;
    stack->top=newSymbol;
    if(nontoken==NT_Nothink){
        newSymbol->token= token
        newSymbol->NonTerminal=NT_Nothink;
    }
    newSymbol->notTerminal=nontoken;

}

void StackPop (ParserStack* stack){
    StackSymbol* temp= ParserStack->top;
    ParserStack->top = ParserStack->top->next;
    free(temp);
}

void StackFree (ParserStack* stack){
    StackSymbol *tmp = stack->top;
    StackSymbol *temp;
    while(tmp->next!=NULL){
        temp=tmp->next;
        free(tmp);
        tmp=temp;
    }    
    free(temp);
    stack->top=NULL;
}