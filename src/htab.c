// htab.c
// Autor: Martin Knotek, FIT
// Kod prebran a upraven z projektu do IJC


#include "htab_private.h"

#define FNV_PRIME 16777619u
#define OFFSET_BASIS 2166136261u
#define PRIMES_COUNT 27
#define PRIMES_VALUES { \
  37, 79, 163, 331, 673, 1361, 2729, 5471, 10949, \
  21911, 43853, 87719, 175447, 350899, 701819, 1403641, \
  2807303, 5614657, 11229331, 22458671, 44917381, 89834777, \
  179669557, 359339171, 718678369, 1437356741, 2874713497 \
}

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
// n is the capacity, table is allocate ~2times bigger
htab_t* htabCreate(uint32_t n, size_t dataSize) {
  htab_t* t = malloc(sizeof(htab_t));
  if(t) {
    size_t itemSize = dataSize + sizeof(htabItem_t);
    uint32_t capacity = findCapacity(n);
    t->table = malloc(capacity * itemSize);
    if(!t->table) {
      free(t);
      return NULL;
    }
    t->capacity = capacity;
    t->size = 0;
    t->threshold = (capacity*3)>>2;
    t->itemSize = itemSize;
    t->dataSize = dataSize;
    htabItem_t* item = t->table;
    for(uint32_t i = 0; i < capacity; i++) {
      item->key = NULL;
      item = (htabItem_t*)((char*)item + itemSize);
    }
  }
  return t;
}

// Wipes table clean
void htabWipe(htab_t* t) {
  htabItem_t* item = t->table;
  for(uint32_t i = 0; i < t->capacity; i++) {
    free(item->key);
    item->key = NULL;
    item = (htabItem_t*)((char*)item + t->itemSize);
  }
  t->size = 0;
}

// Destroys table
void htabDestroy(htab_t* t) {
  htabItem_t* item = t->table;
  for(uint32_t i = 0; i < t->capacity; i++) {
    free(item->key);
    item = (htabItem_t*)((char*)item + t->itemSize);
  }
  free(t->table);
  free(t);
}

// Executes f on each item in table (empty included)
void htabForEach(htab_t* t, void (*f)(htabItem_t* item)) {
  htabItem_t* item = t->table;
  for(uint32_t i = 0; i < t->capacity; i++) {
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
// exits on fail
htabItem_t* htabWrite(htab_t* t, char* key, void* data) {
  htabItem_t* item = htabFindKey(t, key);
  if(!item->key) {
    // doble size when full
    if(t->size >= t->threshold) {
      htabResize(t, t->capacity<<1);
      item = htabFindKey(t, key);
    }
    // create entry
    item->key = malloc(strlen(key) + 1);
    if(!item->key) exit(EXIT_FAILURE);
    // fill entry
    strcpy(item->key, key);
    item->hash = hashFunction(key, OFFSET_BASIS);
    t->size++;
  }
  memcpy(item->data, data, t->dataSize);
  return item;
}

// Writes data to table
// returns NULL if entry already exists
// exits on fail
htabItem_t* htabWriteNew(htab_t* t, char* key, void* data) {
  htabItem_t* item = htabFindKey(t, key);
  if(item->key) return NULL;
  // double size when full
  if(t->size >= t->threshold) {
    htabResize(t, t->capacity<<1);
    item = htabFindKey(t, key);
  }
  // create entry
  item->key = malloc(strlen(key) + 1);
  if(!item->key) exit(EXIT_FAILURE);
  // fill entry
  strcpy(item->key, key);
  item->hash = hashFunction(key, OFFSET_BASIS);
  t->size++;
  memcpy(item->data, data, t->dataSize);
  return item;
}

// Returns pointer to data of given key
htabItem_t* htabGet(htab_t* t, char* key) {
  htabItem_t* item = htabFindKey(t, key);
  // Check if the cell is empty
  if(item->key == NULL) return NULL;
  return item;
}

uint32_t findCapacity(uint32_t size) {
  static const uint32_t primes[] = PRIMES_VALUES;
  for(unsigned i = 0; i < PRIMES_COUNT; i++) {
    if (primes[i] > size) return primes[i];
  }
  return size += (size % 1) + 1; // return bigger odd nuber when prime lookup fails
}

htabItem_t* htabResize(htab_t* t, uint32_t size) {
  uint32_t capacity = findCapacity(size);
  uint32_t threshold = (capacity*3)>>2;

  htab_t* new = htabCreate(capacity, t->dataSize);
  if(!new) exit(EXIT_FAILURE); // chyba ukoncit

  // copy all items from old to new
  htabItem_t* item = t->table;
  for(uint32_t i = 0; i < t->capacity; i++) {
    if(item->key) {
      // find new location
      htabItem_t* dst = htabFindIdx(new, item->hash % capacity);
      // if full rehash
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
  t->threshold = threshold;
  free(new);
  return t->table;
}