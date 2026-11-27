#!/bin/bash

# Soubor vygenerovan AI! mirne upraven
# https://share.gemini.google/5d3e7oRA4geQ


# Exit immediately if a command exits with a non-zero status
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

HTAB_SRC="../src/htab.c"
TEST_SRC="test_htab.c"
EXECUTABLE="./htab_test"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Starting Hash Table Test Suite...${NC}"

# Check if source files exist
if [ ! -f "$HTAB_SRC" ] || [ ! -f "$TEST_SRC" ]; then
    echo -e "${RED}Error: Source files missing! Ensure $HTAB_SRC and $TEST_SRC are in the current directory.${NC}"
    exit 1
fi

rm -f "$EXECUTABLE"
echo "Compiling..."
if ! gcc -Wall -Wextra -g -O0 "$HTAB_SRC" "$TEST_SRC" -o "$EXECUTABLE"
then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi
echo -e "${GREEN}Compilation successful.${NC}"

echo "Running tests..."
if "$EXECUTABLE"
then
    echo -e "${GREEN}SUCCESS: Logic tests passed.${NC}"
else
    echo -e "${RED}FAILURE: Logic tests failed.${NC}"
    exit 1
fi

# Memory Leak check with Valgrind (if available)
if command -v valgrind &> /dev/null; then
    echo -e "${YELLOW}Running Valgrind memory checks...${NC}"
    if valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 -q "$EXECUTABLE"
    then
        echo -e "${GREEN}SUCCESS: No memory leaks detected.${NC}"
    else
        echo -e "${RED}FAILURE: Memory leaks or errors detected by Valgrind!${NC}"
    fi
else
    echo -e "${YELLOW}Notice: 'valgrind' not found. Skipping memory leak checks.${NC}"
fi

echo "Cleaning up..."
rm -f "$EXECUTABLE"

echo -e "${GREEN}--- All checks complete! ---${NC}"