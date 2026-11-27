// Implementace překladače imperativního jazyka IFJ26
// htab_private.h
// Interní definice hashovací tabulky

// Autor: Martin Knotek (xknotem00)

#ifndef HTAB_PRIVATE_H
#define HTAB_PRIVATE_H


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "htab.h"

uint32_t hashFunction(char* key, uint32_t offset);

htab_t* htabCreate(uint32_t n);
void htabWipe(htab_t* t);
void htabDestroy(htab_t* t);

htabItem_t* htabFindIdx(htab_t* t, uint32_t i);
htabItem_t* htabFindKey(htab_t* t, char* key);

htabItem_t* htabWrite(htab_t* t, char* key);
htabItem_t* htabWriteNew(htab_t* t, char* key);
htabItem_t* htabGet(htab_t* t, char* key);
uint32_t findCapacity(uint32_t size);
htabItem_t* htabResize(htab_t* t, uint32_t capacity);

#endif