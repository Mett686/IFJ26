// Vygenerovano AI! pote upraveno
// https://share.gemini.google/5d3e7oRA4geQ

#include <stdio.h>
#include <assert.h>


#include "../src/htab_private.h"

int main() {
    printf("  [TEST] htabCreate...\n");
    htab_t* table = htabCreate(5);
    assert(table != NULL);
    assert(table->capacity > 5);
    assert(table->size == 0);
    assert(table->parent == NULL);

    printf("  [TEST] htabWrite (new keys)...\n");
    htabItem_t* item1 = htabWrite(table, "testKey1");
    htabItem_t* item2 = htabWrite(table, "testKey2");
    item1->isMutable = true;
    item1->returnType = 42;
    item2->isMutable = false;
    item2->returnType = 84;
    assert(table->size == 2);

    printf("  [TEST] htabGet (existing keys)...\n");
    htabItem_t* res1 = htabGet(table, "testKey1");
    htabItem_t* res2 = htabGet(table, "testKey2");
    assert(res1 != NULL && res1->isMutable);
    assert(res1->returnType == 42);
    assert(res2 != NULL && !res2->isMutable);
    assert(res2->returnType == 84);

    printf("  [TEST] htabGet (non-existent key)...\n");
    int* res3 = (int*)htabGet(table, "notFound");
    assert(res3 == NULL);

    printf("  [TEST] htabWrite (overwrite existing key)...\n");
    htabItem_t* res4 = htabWrite(table, "testKey1");
    res4->isMutable = false;
    res4->returnType = 999;
    assert(table->size == 2); // Size should remain the same
    res4 = htabGet(table, "testKey1");
    assert(res4 != NULL && res4->returnType == 999);
    assert(!res4->isMutable);

    printf("  [TEST] htabWriteNew (new key)...\n");
    htabItem_t* res5 = htabWriteNew(table, "testKey3");
    assert(res5 != NULL);
    res5->isMutable = true;
    res5->returnType = 42;

    printf("  [TEST] htabWriteNew (existing key)...\n");
    assert(!htabWriteNew(table, "testKey3"));
    assert(htabGet(table, "testKey3")->returnType == 42);

    printf("  [TEST] htabWrite(New) (table space check)...\n");
    assert(htabWrite(table, "testKey4") != NULL);
    assert(htabWrite(table, "testKey4") != NULL);

    printf("  [TEST] htabResize (valid resize)...\n");
    uint32_t oldCapacity = table->capacity;
    htabResize(table, 10);
    assert(table->capacity >= oldCapacity);
    assert(htabGet(table, "testKey3")->returnType == 42);

    printf("  [TEST] htabResize (invalid resize)...\n");
    uint32_t capacity = table->capacity;
    assert(!htabResize(table, 2));
    assert(table->capacity == capacity);
    assert(htabGet(table, "testKey3")->returnType == 42);

    printf("  [TEST] htabWipe...\n");
    htabWipe(table);
    assert(table->size == 0);
    assert(htabGet(table, "testKey2") == NULL);

    printf("  [TEST] htabDestroy...\n");
    htabDestroy(table); 

    printf("  All assertions passed!\n");
    return 0;
}