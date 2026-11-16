#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>

//https://share.gemini.google/nhvyD0ViyAyR

// TODO: Include your actual project headers here
#include "../src/htab.h"
//#include "../src/ast.h"     // Header containing tnode, opAdd, etc.
#include "../src/abtDecoding.h" // Header containing checkNode()
#include "../src/mock/mock_ast.h"  //fix later

// --- Mocks for isolated testing ---
// Since checkNode uses 'inBlock' and 'DLLInsertLast', we mock them 
// here so the file compiles without needing the rest of the compiler.
bool inBlock = false; 
void DLLInsertLast(int a, void* data) { 
    // No-op mock for testing
}

// --- Test Cases ---

void test_opAdd_same_type() {
    printf("Running test_opAdd_same_type...\n");
    
    // Set up dummy nodes
    tnode leftNode =  { .dataType = 1 /* TYPE_INT */, .value = NULL };
    tnode rightNode = { .dataType = 1 /* TYPE_INT */, .value = NULL };
    tnode parent =    { .dataType = opAdd /* Ensure this matches your enum */, 
                        .left = &leftNode, 
                        .right = &rightNode };

    // Should successfully propagate TYPE_INT to the parent
    //checkNode(&parent, NULL);
    semanticCheck(&parent, NULL);
    
    assert(parent.dataType == 1);
    printf("test_opAdd_same_type passed!\n");
}

void test_type_mismatch_exits_with_6() {
    printf("Running test_type_mismatch_exits_with_6...\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // --- Child Process ---
        // Setup mismatched types
        tnode leftNode =  { .dataType = 1 /* TYPE_INT */ };
        tnode rightNode = { .dataType = 2 /* TYPE_FLOAT */ };
        tnode parent =    { .dataType = opAdd, 
                            .left = &leftNode, 
                            .right = &rightNode };
        
        // This should trigger exit(6)
        semanticCheck(&parent, NULL);
        
        // If we reach here, the test failed
        exit(0); 
    } else {
        // --- Parent Process ---
        int status;
        waitpid(pid, &status, 0);
        
        // Assert the child exited normally (via exit()) and returned 6
        assert(WIFEXITED(status));
        assert(WEXITSTATUS(status) == 6);
        printf("test_type_mismatch_exits_with_6 passed!\n");
    }
}

int main() {
    printf("Starting Semantic Analyzer Tests...\n\n");
    
    test_opAdd_same_type();
    test_type_mismatch_exits_with_6();
    
    // Add tests for opDeclare, opAssign, opBlock here
    
    printf("\nAll tests passed successfully!\n");
    return 0;
}