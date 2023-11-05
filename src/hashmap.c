#include "hashmap.h"

// Hashes the given key using the secret key in the HashMap using SipHash-2-4
// with an 8 byte output and stores the result in hash.
static void hashmap_hash_key(const HashMap *map, const char *key,
                             uint64_t *hash) {
  if (map == NULL || key == NULL || hash == NULL) {
    return;
  }

  uint8_t hash_bytes[8] = {0};
  siphash(key, strlen(key), map->key, hash_bytes, 8);

  *hash = 0;
  for (size_t i = 0; i < 8; i++) {
    *hash |= ((uint64_t)hash_bytes[i]) << (i * 8);
  }
}

// Grows the HashMap to the given number of buckets. Will not shrink the
// HashMap.
static void hashmap_grow(HashMap *map, size_t new_buckets) {
  if (map == NULL || map->buckets >= new_buckets) {
    return;
  }

  map->buckets = new_buckets;
  map->entries = realloc(map->entries, new_buckets * sizeof(*(map->entries)));
  for (size_t i = map->buckets; i < new_buckets; i++) {
    map->entries[i] = NULL;
  }
}

// If the HashMap has no buckets, sets the number of buckets to 8. If the load
// factor is greater than 0.75, doubles the number of buckets.
static void hashmap_grow_if_needed(HashMap *map) {
  if (map == NULL) {
    return;
  }

  if (map->buckets == 0) {
    hashmap_grow(map, 8);
  } else if (map->len * 4 >= map->buckets * 3) {
    // if the load factor is greater than 0.75, double the number of buckets
    hashmap_grow(map, map->buckets * 2);
  }
}

// Creates a new HashMap with 0 buckets. Will be resized to 8 buckets when the
// first entry is added.
HashMap *hashmap_new() {
  HashMap *map = calloc(1, sizeof(*map));

  if (map == NULL) {
    return NULL;
  }

  map->len = 0;
  map->buckets = 0;
  map->entries = NULL;

  uint8_t key[16] = {0};
  for (size_t i = 0; i < 16; i++) {
    key[i] = rand() % (UINT8_MAX + 1);
  }
  memcpy(map->key, key, 16);

  return map;
}

// Creates a new HashMap with the given number of buckets.
HashMap *hashmap_with_buckets(size_t buckets) {
  HashMap *map = hashmap_new();

  if (map == NULL) {
    return NULL;
  }

  map->buckets = buckets;
  map->entries = calloc(buckets, sizeof(*(map->entries)));
  for (size_t i = 0; i < buckets; i++) {
    map->entries[i] = NULL;
  }

  return map;
}

// Sets the value for the given key. If the key already exists, the old value is
// returned. Takes ownership of the value and frees it when the map is freed.
// The key is copied and no ownership is taken.
void *hashmap_insert(HashMap *map, const char *key, void *value) {
  hashmap_grow_if_needed(map);
  if (map->entries == NULL) {
    return NULL;
  }

  uint64_t hash = 0;
  hashmap_hash_key(map, key, &hash);

  size_t bucket = hash % map->buckets;
  Entry **entry = map->entries + bucket;
  while (*entry != NULL) {
    if (strcmp((*entry)->key, key) == 0) {
      void *old_value = (*entry)->value;
      (*entry)->value = value;
      return old_value;
    }

    entry = &(*entry)->next;
  }

  *entry = calloc(1, sizeof(**entry));
  (*entry)->key = strdup(key);
  (*entry)->value = value;
  (*entry)->next = NULL;

  return NULL;
}

// Returns the value for the given key. If the key does not exist, NULL is
// returned.
void *hashmap_get(const HashMap *map, const char *key) {
  if (map == NULL || map->entries == NULL) {
    return NULL;
  }

  uint64_t hash = 0;
  hashmap_hash_key(map, key, &hash);

  size_t bucket = hash % map->buckets;
  Entry *entry = map->entries[bucket];
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      return entry->value;
    }

    entry = entry->next;
  }

  return NULL;
}

// Deletes the entry for the given key and returns the value, handing ownership
// of the value to the caller. Returns NULL if the key does not exist.
void *hashmap_remove(HashMap *map, const char *key) {
  if (map == NULL || map->entries == NULL) {
    return NULL;
  }

  uint64_t hash = 0;
  hashmap_hash_key(map, key, &hash);

  size_t bucket = hash % map->buckets;
  Entry **entry = map->entries + bucket;
  while (*entry != NULL) {
    if (strcmp((*entry)->key, key) == 0) {
      void *value = (*entry)->value;
      Entry *next = (*entry)->next;
      free((*entry)->key);
      free(*entry);
      *entry = next;
      return value;
    }

    entry = &(*entry)->next;
  }

  return NULL;
}

// Frees the HashMap and all its entries, setting the pointer to map to NULL.
// Will also free all keys and values.
void hashmap_free(HashMap **map) {
  if (map == NULL || *map == NULL) {
    return;
  }

  for (size_t i = 0; i < (*map)->buckets; i++) {
    Entry *entry = (*map)->entries[i];
    while (entry != NULL) {
      Entry *next = entry->next;
      free(entry->key);
      free(entry->value);
      free(entry);
      entry = next;
    }
  }

  free((*map)->entries);
  free(*map);
  *map = NULL;
}
