#ifndef MOCK_AST_H
#define MOCK_AST_H

// Add EVERY operation your semantic analyzer checks for in here!
typedef enum {
    opAdd = 100, 
    opDec,
    opAssign,
    opBlock,
    opSub, 
    opMul,
    opDiv,
    opMod,
    opEqual,
    opLesser,
    opBigger,
    opCall,
    opDeclare,
    opEndBlock
} NodeType;

typedef struct tnode {
    int dataType;          
    NodeType type;         
    void* value;           
    struct tnode* left;    
    struct tnode* right;   
    bool isVar;
} tnode;

#endif