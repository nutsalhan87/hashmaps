#ifndef HASHMAPS_HASHMAP_SC_H
#define HASHMAPS_HASHMAP_SC_H

#include <stdbool.h>
#include <stdint.h>

struct hashmap_sc;

struct hashmap_sc *hashmap_sc_new(uint64_t (*hasher)(uint64_t), void (*value_free)(void *));

bool hashmap_sc_insert(struct hashmap_sc *self, uint64_t key, void *value);

void *hashmap_sc_find(struct hashmap_sc *self, uint64_t key);

bool hashmap_sc_delete(struct hashmap_sc *self, uint64_t key);

void hashmap_sc_clear(struct hashmap_sc *self);

void hashmap_sc_free(struct hashmap_sc *self);

#endif // HASHMAPS_HASHMAP_SC_H