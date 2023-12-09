#include "siphash/siphash.h"

#include <stdlib.h>
#include <string.h>

#pragma once

// Entry in the HashMap that acts as a node in a linked list.
typedef struct Entry {
  char *key;
  void *value;
  struct Entry *next;
} Entry;

// HashMap that uses SipHash-2-4 to hash keys. Collisions are resolved by
// chaining in a linked list.
typedef struct HashMap {
  size_t len;
  size_t buckets;
  uint8_t key[16];
  Entry **entries;
} HashMap;

// Creates a new HashMap with 0 buckets. Will be resized to 8 buckets when the
// first entry is added.
HashMap *hashmap_new();

// Creates a new HashMap with the given number of buckets.
HashMap *hashmap_with_buckets(size_t buckets);

// Returns the number of keys stored in the HashMap.
size_t hashmap_len(const HashMap *map);

// Returns the ratio of keys to buckets in the HashMap.
double hashmap_load_factor(const HashMap *map);

// Sets the value for the given key. If the key already exists, the old value is
// returned. Takes ownership of the value and frees it when the map is freed.
// The key is copied and no ownership is taken.
void *hashmap_insert(HashMap *map, const char *key, void *value);

// Returns the value for the given key. If the key does not exist, NULL is
// returned.
void *hashmap_get(const HashMap *map, const char *key);

// Deletes the entry for the given key and returns the value, handing ownership
// of the value to the caller. Returns NULL if the key does not exist.
void *hashmap_remove(HashMap *map, const char *key);

// Frees the HashMap and all its entries, setting the pointer to map to NULL.
// Will also free all keys and values.
void hashmap_free(HashMap **map);
