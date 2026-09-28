// htab_private.h
// Autor: Martin Knotek, FIT

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "htab.h"

uint32_t hashFunction(char* key, uint32_t offset);

htab_t* htabCreate(size_t n, size_t dataSize);
void htabWipe(htab_t* t);
void htabDestroy(htab_t* t);

void htabForEach(htab_t* t, void (*f)(htabItem_t* item));

htabItem_t* htabFindIdx(htab_t* t, uint32_t i);
htabItem_t* htabFindKey(htab_t* t, char* key);

htabItem_t* htabWrite(htab_t* t, char* key, void* data);
htabItem_t* htabWriteNew(htab_t* t, char* key, void* data);
void* htabGet(htab_t* t, char* key);
htabItem_t* htabResize(htab_t* t, uint32_t capacity);