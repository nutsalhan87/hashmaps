#ifndef HASHMAPS_HASHMAP_DH_H
#define HASHMAPS_HASHMAP_DH_H

#include <stdbool.h>
#include <stdint.h>

struct hashmap_dh;

struct hashmap_dh *hashmap_dh_new(uint64_t (*hasher1)(uint64_t), uint64_t (*hasher2)(uint64_t), void (*value_free)(void *));

bool hashmap_dh_insert(struct hashmap_dh * self, uint64_t key, void *value);

void *hashmap_dh_find(struct hashmap_dh *self, uint64_t key);

bool hashmap_dh_delete(struct hashmap_dh *self, uint64_t key);

void hashmap_dh_clear(struct hashmap_dh *self);

void hashmap_dh_free(struct hashmap_dh *self);

#endif // HASHMAPS_HASHMAP_DH_H
