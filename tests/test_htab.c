// Vygenerovano AI! pote upraveno
// https://share.gemini.google/5d3e7oRA4geQ

#include <stdio.h>
#include <assert.h>


#include "../src/htab.h"
#include "../src/abtDecoding.h"

int main() {
    printf("  [TEST] htabCreate...\n");
    htab_t* table = htabCreate(5, sizeof(int));
    assert(table != NULL);
    assert(table->capacity == 5);
    assert(table->size == 0);
    assert(table->dataSize == sizeof(int));

    printf("  [TEST] htabWrite (new keys)...\n");
    int val1 = 42;
    int val2 = 84;
    htabWrite(table, "testKey1", &val1);
    htabWrite(table, "testKey2", &val2);
    assert(table->size == 2);

    printf("  [TEST] htabGet (existing keys)...\n");
    int* res1 = (int*)htabGet(table, "testKey1");
    int* res2 = (int*)htabGet(table, "testKey2");
    assert(res1 != NULL && *res1 == 42);
    assert(res2 != NULL && *res2 == 84);

    printf("  [TEST] htabGet (non-existent key)...\n");
    int* res3 = (int*)htabGet(table, "notFound");
    assert(res3 == NULL);

    printf("  [TEST] htabWrite (overwrite existing key)...\n");
    int val3 = 999;
    htabWrite(table, "testKey1", &val3);
    assert(table->size == 2); // Size should remain the same
    int* res4 = (int*)htabGet(table, "testKey1");
    assert(res4 != NULL && *res4 == 999);

    printf("  [TEST] htabWriteNew (new key)...\n");
    htabWriteNew(table, "testKey3", &val1);
    int* res5 = (int*)htabGet(table, "testKey3");
    assert(res5 && *res5 == val1);

    printf("  [TEST] htabWriteNew (existing key)...\n");
    assert(!htabWriteNew(table, "testKey3", &val3));
    assert(*(int*)htabGet(table, "testKey3") == val1);

    printf("  [TEST] htabWrite(New) (table space check)...\n");
    assert(!htabWrite(table, "testKey4", &val1));
    assert(!htabWrite(table, "testKey4", &val1));

    printf("  [TEST] htabResize (valid resize)...\n");
    htabResize(table, 10);
    assert(table->capacity == 10);
    assert(*(int*)htabGet(table, "testKey3") == val1);

    printf("  [TEST] htabResize (invalid resize)...\n");
    assert(!htabResize(table, 2));
    assert(*(int*)htabGet(table, "testKey3") == val1);

    printf("  [TEST] htabWipe...\n");
    htabWipe(table);
    assert(table->size == 0);
    assert(htabGet(table, "testKey2") == NULL);

    printf("  [TEST] htabDestroy...\n");
    htabDestroy(table); 

    printf("  All assertions passed!\n");
    return 0;
}