// htab.h
// Autor: Martin Knotek, FIT

typedef struct htabItem {
  char* key;
  uint32_t hash;
  int nested;
  char *string;
  bool isMutable;
  bool isFunction;
  int paramCount;
  int *paramTypes;
  int returnType;
  char data[];
} htabItem_t;

typedef struct htab {
  uint32_t capacity;
  uint32_t size;
  uint32_t threshold;
  size_t itemSize;
  size_t dataSize;
  htabItem_t* table;
  struct htab *parent;
} htab_t;

// Creates and initializes table
htab_t* htabCreate(uint32_t n, size_t dataSize);

// Wipes table clean
void htabWipe(htab_t* t);

// Destroys table
void htabDestroy(htab_t* t);

// Writes data to table, overwrites existing entries
// returns pointer to the written item
htabItem_t* htabWrite(htab_t* t, char* key, void* data);

// Inserts new key into htab, returns NULL if entry already exists
htabItem_t* htabWriteNew(htab_t* t, char* key, void* data);

// Returns pointer to data of given key
htabItem_t* htabGet(htab_t* t, char* key);