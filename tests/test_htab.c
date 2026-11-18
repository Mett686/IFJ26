// Vygenerovano AI! pote upraveno
// https://share.gemini.google/5d3e7oRA4geQ

#include <stdio.h>
#include <assert.h>


#include "../src/htab_private.h"

int main() {

  // should create table of size 7
  printf("  [TEST] htabCreate...\n");
  htab_t* table = htabCreate(5);
  assert(table != NULL);
  assert(table->table != NULL);
  assert(table->capacity == 7);
  assert(table->size == 0);
  assert(table->parent == NULL);

  // inserts two new items with values and checks size
  printf("  [TEST] htabWrite (new keys)...\n");
  htabItem_t* item1 = htabWrite(table, "testKey1");
  htabItem_t* item2 = htabWrite(table, "testKey2");
  item1->isMutable = true;
  item1->returnType = 42;
  item2->isMutable = false;
  item2->returnType = 84;
  assert(table->size == 2);
  
  // tests readability of written items
  printf("  [TEST] htabGet (existing keys)...\n");
  htabItem_t* res1 = htabGet(table, "testKey1");
  htabItem_t* res2 = htabGet(table, "testKey2");
  assert(res1 != NULL && res1->isMutable);
  assert(res1->returnType == 42);
  assert(res2 != NULL && !res2->isMutable);
  assert(res2->returnType == 84);
  
  // tests retrieving non existent key
  printf("  [TEST] htabGet (non-existent key)...\n");
  int* res3 = (int*)htabGet(table, "notFound");
  assert(res3 == NULL);
  
  // tests writing new data to existing key
  printf("  [TEST] htabWrite (overwrite existing key)...\n");
  htabItem_t* res4 = htabWrite(table, "testKey1");
  res4->isMutable = false;
  res4->returnType = 999;
  assert(table->size == 2); // Size should remain the same
  res4 = htabGet(table, "testKey1");
  assert(res4 != NULL && res4->returnType == 999);
  assert(!res4->isMutable);
  
  // tests writing to new key
  printf("  [TEST] htabWriteNew (new key)...\n");
  htabItem_t* res5 = htabWriteNew(table, "testKey3");
  assert(res5 != NULL);
  res5->isMutable = true;
  res5->returnType = 42;
  
  // tests overwriting existing key with writeNew
  printf("  [TEST] htabWriteNew (existing key)...\n");
  assert(!htabWriteNew(table, "testKey3"));
  assert(htabGet(table, "testKey3")->returnType == 42);
  
  // test over-filling table
  printf("  [TEST] htabWrite(New) (table space check)...\n");
  assert(htabWrite(table, "testKey4") != NULL);
  assert(htabWrite(table, "testKey5") != NULL);
  assert(htabWrite(table, "testKey6") != NULL);
  // capacity should be next in PRIMES_VALUES so 17
  assert(table->capacity == 17);
  assert(table->size == 6);

  // test resize
  printf("  [TEST] htabResize (valid resize bigger)...\n");
  assert(htabResize(table, 20));
  assert(table->capacity == 37);
  
  // test wiping table
  printf("  [TEST] htabWipe...\n");
  htabWipe(table);
  assert(table->size == 0);
  assert(htabGet(table, "testKey2") == NULL);

  // test resizing to smaller
  printf("  [TEST] htabResize (valid resize smaller)...\n");
  assert(htabResize(table, 10));
  assert(table->capacity == 17);
  
  // test resizing to too small size
  printf("  [TEST] htabResize (invalid resize)...\n");
  // fill table again
  assert(htabWrite(table, "testKey1") != NULL);
  assert(htabWrite(table, "testKey2") != NULL);
  assert(htabWrite(table, "testKey3") != NULL);
  assert(htabWrite(table, "testKey4") != NULL);
  assert(htabWrite(table, "testKey5") != NULL);
  assert(htabWrite(table, "testKey6") != NULL);
  assert(table->size == 6);
  // try to resize down
  uint32_t capacity = table->capacity;
  assert(!htabResize(table, 6));
  assert(table->capacity == capacity);
  assert(table->size == 6);
  
  printf("  [TEST] htabDestroy...\n");
  htabDestroy(table); 
  
  printf("  All assertions passed!\n");
  return 0;
}