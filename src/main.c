#include "hashmap.h"

#include <stdio.h>
#include <string.h>

int main(void) {
  HashMap *map = hashmap_new();
  char *value1 = strdup("Hello, World!");
  char *value2 = strdup("hello world");
  char *value3 = strdup("value");

  hashmap_insert(map, "key1", value1);
  printf("key1: %s\n", (char *)hashmap_get(map, "key1"));

  value1 = hashmap_insert(map, "key1", value2);
  if (value1 != NULL) {
    free(value1);
  }

  printf("key1: %s\n", (char *)hashmap_get(map, "key1"));

  hashmap_insert(map, "key2", value3);
  printf("key2: %s\n", (char *)hashmap_get(map, "key2"));

  void *key1 = hashmap_remove(map, "key1");
  free(key1);

  hashmap_free(&map);

  return 0;
}
