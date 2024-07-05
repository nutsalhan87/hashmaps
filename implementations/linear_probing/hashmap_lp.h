#ifndef HASHMAPS_HASHMAP_LP_H
#define HASHMAPS_HASHMAP_LP_H

#include <stdbool.h>
#include <stdint.h>

struct hashmap_lp;

struct hashmap_lp *hashmap_lp_new(uint64_t (*hasher)(uint64_t), void (*value_free)(void *));

bool hashmap_lp_insert(struct hashmap_lp * self, uint64_t key, void *value);

void *hashmap_lp_find(struct hashmap_lp *self, uint64_t key);

bool hashmap_lp_delete(struct hashmap_lp *self, uint64_t key);

void hashmap_lp_clear(struct hashmap_lp *self);

void hashmap_lp_free(struct hashmap_lp *self);

#endif // HASHMAPS_HASHMAP_LP_H