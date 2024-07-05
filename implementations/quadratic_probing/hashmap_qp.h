#ifndef HASHMAPS_HASHMAP_QP_H
#define HASHMAPS_HASHMAP_QP_H

#include <stdbool.h>
#include <stdint.h>

struct hashmap_qp;

struct hashmap_qp *hashmap_qp_new(uint64_t (*hasher)(uint64_t), void (*value_free)(void *));

bool hashmap_qp_insert(struct hashmap_qp * self, uint64_t key, void *value);

void *hashmap_qp_find(struct hashmap_qp *self, uint64_t key);

bool hashmap_qp_delete(struct hashmap_qp *self, uint64_t key);

void hashmap_qp_clear(struct hashmap_qp *self);

void hashmap_qp_free(struct hashmap_qp *self);

#endif // HASHMAPS_HASHMAP_QP_H
