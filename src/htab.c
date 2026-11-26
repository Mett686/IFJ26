// htab.c
// Autor: Martin Knotek, FIT
// Kod prebran a upraven z projektu do IJC


#include "htab_private.h"

#define FNV_PRIME 16777619u
#define OFFSET_BASIS 2166136261u
#define PRIMES_COUNT 27
#define PRIMES_VALUES { \
  7, 17, 37, 79, 163, 331, 673, 1361, 2729, 5471, 10949, \
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
htab_t* htabCreate(uint32_t n) {
  htab_t* t = malloc(sizeof(htab_t));
  static unsigned tableId = 0;
  if(t) {
    uint32_t capacity = findCapacity(n);
    t->table = malloc(capacity * sizeof(htabItem_t));
    if(!t->table) {
      free(t);
      return NULL;
    }
    t->capacity = capacity;
    t->size = 0;
    t->threshold = (capacity*3)>>2;
    t->tableId = tableId;
    tableId++;
    t->parent = NULL;
    for(uint32_t i = 0; i < capacity; i++) {
      memset(&t->table[i], 0, sizeof(htabItem_t));
    }
  }
  return t;
}

// Wipes table clean
void htabWipe(htab_t* t) {
  for(uint32_t i = 0; i < t->capacity; i++) {
    free(t->table[i].key);
    memset(&t->table[i], 0, sizeof(htabItem_t));
  }
  t->size = 0;
}

// Destroys table
void htabDestroy(htab_t* t) {
  for(uint32_t i = 0; i < t->capacity; i++) {
    free(t->table[i].key);
  }
  free(t->table);
  free(t);
}

// Returns pointer to item with given index
htabItem_t* htabFindIdx(htab_t* t, uint32_t i) {
  htabItem_t* item = NULL;
  if(i < t->capacity) {
    item = &t->table[i];
  }
  return item;
}

// Returns pointer to item with given key or first empty
htabItem_t* htabFindKey(htab_t* t, char* key) {
  const uint32_t h = hashFunction(key, OFFSET_BASIS);
  htabItem_t* item = &t->table[h % t->capacity];

  // if match is not found look further
  if(item->key && strcmp(key, item->key)) {
    uint32_t newHash = h;
    do {
      newHash = hashFunction(key, newHash);
      item = &t->table[newHash % t->capacity];
    } while(item->key && strcmp(key, item->key));
  }
  return item;
}

// Writes data to table, overwrites existing entries
// exits on fail
htabItem_t* htabWrite(htab_t* t, char* key) {
  htabItem_t* item = htabFindKey(t, key);
  if(!item->key) {
    // doble size when full
    if(t->size >= t->threshold) {
      htabResize(t, t->capacity<<1);
      item = htabFindKey(t, key);
    }
    // create entry
    item->key = malloc(strlen(key) + 1);
    if(!item->key) exit(99);
    // fill entry
    strcpy(item->key, key);
    item->hash = hashFunction(key, OFFSET_BASIS);
    t->size++;
  }
  return item;
}

// Writes data to table
// returns NULL if entry already exists
// exits on fail
htabItem_t* htabWriteNew(htab_t* t, char* key) {
  htabItem_t* item = htabFindKey(t, key);
  if(item->key) return NULL;
  // double size when full
  if(t->size >= t->threshold) {
    htabResize(t, t->capacity<<1);
    item = htabFindKey(t, key);
  }
  // create entry
  item->key = malloc(strlen(key) + 1);
  if(!item->key) exit(99);
  // fill entry
  strcpy(item->key, key);
  item->hash = hashFunction(key, OFFSET_BASIS);
  t->size++;
  return item;
}

// Returns pointer to item with given key
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

// resizes table to closest bigger prime
// when resizing to smaller table needs half of the space to be empty
htabItem_t* htabResize(htab_t* t, uint32_t size) {
  if(size < t->size<<1) return NULL;

  uint32_t capacity = findCapacity(size);
  uint32_t threshold = (capacity*3)>>2;

  htabItem_t* new = malloc(capacity*sizeof(htabItem_t));
  if(!new) exit(99); // chyba ukoncit
  for(uint32_t i = 0; i < capacity; i++) {
    memset(&new[i], 0, sizeof(htabItem_t));
  }

  // copy all items from old to new
  for(uint32_t i = 0; i < t->capacity; i++) {
    htabItem_t* src = &t->table[i];
    if(t->table[i].key) {
      // find new location
      htabItem_t* dst = &new[src->hash % capacity];
      // if full rehash
      if(dst->key) {
        uint32_t newHash = src->hash;
        do {
          newHash = hashFunction(src->key, newHash);
          dst = &new[newHash % capacity];
        } while(dst->key);
      }
      memcpy(dst, src, sizeof(htabItem_t));
    }
  }
  t->capacity = capacity;
  free(t->table);
  t->table = new;
  t->threshold = threshold;
  return t->table;
}