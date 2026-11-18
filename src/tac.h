//https://share.gemini.google/dSIcU1BGrwgF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    OPND_VAR,    // == <var>, user defined
    OPND_CONST,
    // <symb> == OPND_VAR u OPND_CONST
    OPND_TEMP,      // Compiler-generated vars
    OPND_LABEL_LIT, // == <label-literal>
    // <label == OPND_LABEL_LIT u VARstring u CONSTstringlit
} EOperandType;


typedef struct {
    EOperandType type;
    union {
        char* name;     // Used for variables and labels
        int int_val;    // Used for integer constants
        int temp_id;    // Used for temporaries (e.g., 1 for t1)
    };
} Operand;

typedef enum {
    TAC_ADD, TAC_SUB, TAC_MUL, TAC_DIV, TAC_IntDIV,
    TAC_ADDs, TAC_SUBs, TAC_MULs, TAC_DIVs, TAC_IntDIVs, //stack variants
    TAC_AND, TAC_OR, TAC_NOT,
    TAC_ANDs, TAC_ORs, TAC_NOTs, //stack variants
    TAC_LT, TAC_GT, TAC_EQ,
    TAC_LTs, TAC_GTs, TAC_EQs, //stack variants
    
    TAC_MOVE,
    
    // Control Flow
    TAC_JUMP,
    JUMPifEQ, JUMPifNEQ,
    JUMPifEQs, JUMPifNEQs, //stack variants
    TAC_LABEL,     
    
    // Function Calls
    TAC_PARAM,
    TAC_CALL,
    TAC_RETURN,
    TAC_READ, TAC_WRITE, 
    
    //odtud už cilove instrukce, ig nepouzit
    INT2FLOAT, FLOAT2INT, INT2CHAR, STR2INT, FLOAT2STR, INT2STR,
    INT2FLOATs, FLOAT2INTs, INT2CHARs, STR2INTs, FLOAT2STRs, INT2STRs, //stack variants
    CONCAT, STRLEN, GETCHAR, SETCHAR, 
    CREATEFRAME, PUSHFRAME, POPFRAME, 
    DEFVAR, 
    PUSHS, POPS, CLEARS, //stack
    NEWARRAY, ARRAYGET, ARRAYSET,
    TYPE, ISINT, 
    TYPEs, ISINTs, //stack variants
    EXIT,
    debugBREAK, debugDPRINT
} ETACOperationType;

typedef struct TACInstruction {
    ETACOperationType operationType;
    Operand* result;
    Operand* operand1;
    Operand* operand2;
    
    struct TACInstruction* prev;
    struct TACInstruction* next;
} TACInstruction;

typedef struct {
    TACInstruction* head;
    TACInstruction* tail;
    int temp_count; // To generate unique temporary names (t1, t2, ...)
} TACProgram;