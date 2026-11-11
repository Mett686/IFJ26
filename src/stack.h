//autor: xmiklej00
#include <stdio.h>
#include <scanner.h>
#include <stdbool.h>

typedef enum{
NT_Nothink,
NT_Program,
NT_Prolog,NT_DefList,
NT_FuncList, NT_ParametList, NT_Paramet,
NT_Type, NT_Body, NT_StatementList, NT_Statement,
NT_ValInitial, NT_VarIniatial, NT_If_statement, NT_ElsePart,
NT_Branch, NT_WhileLoop, NT_LoopBody, NT_Block, NT_StatementRest, 
NT_ArgumentList, NT_ArgumentRest, NT_Between, NT_ParametListRest,
NT_ReturnRest
}NonTerminal;

typedef struct StackSymbol {
    NonTerminal notTerminal;
    ETokenType token;
    StackSymbol* next;
}StackSymbol;

typedef struct{
    StackSymbol* top;
}ParserStack;

ParserStack StackInIt (ParserStack *stack);
bool StackPush (ParserStack* stack, NonTerminal nontoken ,EtokenType token);
void StackPop (ParserStack* stack);
void StackFree (ParserStack* stack);

