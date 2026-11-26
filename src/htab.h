// htab.h
// Autor: Martin Knotek, FIT

#ifndef HTAB_H
#define HTAB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct htab;
typedef struct htabItem {
  // Vlastnosti htab
  char* key;
  uint32_t hash;
  struct htab* parent;

  int nested;
  bool isMutable;
  bool isFunction;
  int paramCount;
  int *paramTypes;
  int returnType;

  char *string; // hodnota literalu

  unsigned scopecnt; // scopecnt pro codegen
} htabItem_t;

typedef struct htab {
  uint32_t capacity;   // capacity of the table
  uint32_t size;       // number of items in table
  uint32_t threshold;  // when the table is too full
  htabItem_t* table;
  unsigned tableId;

  struct htab *parent;
} htab_t;

// Creates and initializes table
htab_t* htabCreate(uint32_t n);

// Wipes table clean
void htabWipe(htab_t* t);

// Destroys table
void htabDestroy(htab_t* t);

// Writes data to table, overwrites existing entries
// returns pointer to the written item
htabItem_t* htabWrite(htab_t* t, char* key);

// Inserts new key into htab, returns NULL if entry already exists
htabItem_t* htabWriteNew(htab_t* t, char* key);

// Returns pointer to data of given key
htabItem_t* htabGet(htab_t* t, char* key);

#endif