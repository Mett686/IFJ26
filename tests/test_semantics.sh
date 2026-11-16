#!/bin/bash

#https://share.gemini.google/nhvyD0ViyAyR

# Exit immediately if a command exits with a non-zero status
set -e

# Configuration
TEST_FILE="test_semantic.c"
# Replace semantic.c with the actual file containing your analyzer code
SEMANTIC_FILE="../src/abtDecoding.c" 
HTAB_FILE="../src/htab.c"
EXECUTABLE="test_runner"
INCLUDES="-I../src"

echo "Compiling tests..."
gcc -Wall -g $INCLUDES -o $EXECUTABLE $TEST_FILE $SEMANTIC_FILE $HTAB_FILE

echo "Running tests..."
./$EXECUTABLE

echo "Cleaning up..."
rm -f $EXECUTABLE

echo "Done."