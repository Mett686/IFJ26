#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

TEST_FILE="test_semantic.c"
SEMANTIC_FILE="../src/abtDecoding.c"
HTAB_FILE="../src/htab.c"
EXECUTABLE="test_runner"
INCLUDES="-I../src"

cleanup() {
    rm -f "$EXECUTABLE"
}
trap cleanup EXIT

echo "Compiling semantic tests..."
gcc -Wall -Wextra -g $INCLUDES -o "$EXECUTABLE" "$TEST_FILE" "$SEMANTIC_FILE" "$HTAB_FILE"

echo "Running semantic tests..."
./"$EXECUTABLE"