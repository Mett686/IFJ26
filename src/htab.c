// htab.c
// Autor: Martin Knotek, FIT
// Kod prebran a upraven z projektu do IJC


//TODO check malloc
//TODO check table fullness

#include "htab_private.h"

#define FNV_PRIME 16777619u
#define OFFSET_BASIS 2166136261u

// Hash using FNV-1a hash
uint32_t hashFunction(char* key, uint32_t offset) {
  uint32_t h = offset;
  const unsigned char* p;
  for(p=(const unsigned char*)key; *p != '\0'; p++) {
    h = h ^ *p;
    h *= FNV_PRIME;
  }
  return h;
}

// Creates and initializes table
htab_t* htabCreate(size_t n, size_t dataSize) {
  htab_t* t = malloc(sizeof(htab_t));
  if(t) {
    size_t itemSize = dataSize + sizeof(htabItem_t);
    t->table = malloc(n * itemSize);
    if(!t->table) {
      free(t);
      return NULL;
    }
    t->capacity = n;
    t->size = 0;
    t->itemSize = itemSize;
    t->dataSize = dataSize;
    htabItem_t* item = t->table;
    for(uint32_t i = 0; i < n; i++) {
      item->key = NULL;
      item = (htabItem_t*)((char*)item + itemSize);
    }
  }
  return t;
}

// Wipes table clean
void htabWipe(htab_t* t) {
  htabItem_t* item = t->table;
  for(size_t i = 0; i < t->capacity; i++) {
    free(item->key);
    item->key = NULL;
    item = (htabItem_t*)((char*)item + t->itemSize);
  }
  t->size = 0;
}

// Destroys table
void htabDestroy(htab_t* t) {
  htabItem_t* item = t->table;
  for(size_t i = 0; i < t->capacity; i++) {
    free(item->key);
    item = (htabItem_t*)((char*)item + t->itemSize);
  }
  free(t->table);
  free(t);
}

// Executes f on each item in table (empty included)
void htabForEach(htab_t* t, void (*f)(htabItem_t* item)) {
  htabItem_t* item = t->table;
  for(size_t i = 0; i < t->capacity; i++) {
    f(item);
    item = (htabItem_t*)((char*)item + t->itemSize);
  }
}

// Returns pointer to item with given index
htabItem_t* htabFindIdx(htab_t* t, uint32_t i) {
  htabItem_t* item = NULL;
  if(i < t->capacity) {
    item = (htabItem_t*)((char*)t->table + i*t->itemSize);
  }
  return item;
}

// Returns pointer to item with given key or first empty
htabItem_t* htabFindKey(htab_t* t, char* key) {
  const uint32_t h = hashFunction(key, OFFSET_BASIS);
  htabItem_t* item = htabFindIdx(t, h % t->capacity);

  // if match is not found look further
  if(item->key && strcmp(key, item->key)) {
    uint32_t newHash = h;
    do {
      newHash = hashFunction(key, newHash);
      item = htabFindIdx(t, newHash % t->capacity);
    } while(item->key && strcmp(key, item->key));
  }
  return item;
}

// Writes data to table, overwrites existing entries
htabItem_t* htabWrite(htab_t* t, char* key, void* data) {
  htabItem_t* item = htabFindKey(t, key);
  if(!item->key) {
    item->key = malloc(strlen(key) + 1);
    strcpy(item->key, key);
    item->hash = hashFunction(key, OFFSET_BASIS);
    t->size++;
  }
  memcpy(item->data, data, t->dataSize);
  return item;
}

// Writes data to table, returns NULL if entry already exists
htabItem_t* htabWriteNew(htab_t* t, char* key, void* data) {
  htabItem_t* item = htabFindKey(t, key);
  if(!item->key) {
    item->key = malloc(strlen(key) + 1);
    strcpy(item->key, key);
    item->hash = hashFunction(key, OFFSET_BASIS);
    t->size++;
    memcpy(item->data, data, t->dataSize);
  } else {
    return NULL;
  }
  return item;
}

// Returns pointer to data of given key
void* htabGet(htab_t* t, char* key) {
  htabItem_t* item = htabFindKey(t, key);
  // Check if the cell is empty
  if(item->key == NULL) return NULL;
  return (void*)item->data;
}

htabItem_t* htabResize(htab_t* t, uint32_t capacity) {
  if((capacity*3)>>2 < t->size) return NULL; 
  htab_t* new = htabCreate(capacity, t->dataSize);
  if(!new) return NULL;

  htabItem_t* item = t->table;
  for(uint32_t i = 0; i < t->capacity; i++) {
    if(item->key) {
      htabItem_t* dst = htabFindIdx(new, item->hash % capacity);
      
      if(dst->key) {
        uint32_t newHash = item->hash;
        do {
          newHash = hashFunction(item->key, newHash);
          dst = htabFindIdx(new, newHash % capacity);
        } while(dst->key);
      }
      memcpy(dst, item, t->itemSize);
    }
    item = (htabItem_t*)((char*)item + t->itemSize);
  }
  t->capacity = capacity;
  free(t->table);
  t->table = new->table;
  free(new);
  return t->table;
}