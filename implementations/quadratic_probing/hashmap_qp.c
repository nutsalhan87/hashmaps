#include "hashmap_qp.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOAD_FACTOR 0.7
#define C1 1
#define C2 1

struct hashmap_qp {
    uint32_t entries_count;
    uint32_t slots_count;
    struct slot *slots;
    uint64_t distance_limit;

    uint64_t (*hasher)(uint64_t);

    void (*value_free)(void *);
};

enum slot_status {
    vacant = 0,
    occupied,
    released
};

struct slot {
    uint64_t hash;
    uint64_t key;
    void *value;
    enum slot_status status;
};

static const uint64_t tab64[64] = {
        63, 0, 58, 1, 59, 47, 53, 2,
        60, 39, 48, 27, 54, 33, 42, 3,
        61, 51, 37, 40, 49, 18, 28, 20,
        55, 30, 34, 11, 43, 14, 22, 4,
        62, 57, 46, 52, 38, 26, 32, 41,
        50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16, 9, 12,
        44, 24, 15, 8, 23, 7, 6, 5
};

static uint64_t log2_64(uint64_t value) {
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return tab64[((uint64_t) ((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
}

static void resize_map(struct hashmap_qp *self) {
    size_t new_slots_count = 2 * self->slots_count;
    struct slot *new_slots = calloc(new_slots_count, sizeof(struct slot));

    struct slot *old_slots = self->slots;
    for (size_t i = 0; i < self->slots_count; ++i) {
        if (old_slots[i].status != occupied) {
            continue;
        }

        uint64_t new_initial_hash_index = old_slots[i].hash % new_slots_count;
        uint64_t new_hash_index = new_initial_hash_index;
        for (size_t j = 0; new_slots[new_hash_index].status == occupied; ++j) {
            new_hash_index = (new_initial_hash_index + C1 * j + C2 * j * j) % new_slots_count;
        }
        memcpy(new_slots + new_hash_index, old_slots + i, sizeof(struct slot));
    }
    free(old_slots);
    self->slots = new_slots;
    self->slots_count *= 2;
    self->distance_limit = log2_64(new_slots_count);
}

static struct slot *hashmap_qp_find_inner(struct hashmap_qp *self, uint64_t key) {
    size_t hash_index = self->hasher(key) % self->slots_count;
    struct slot *slot;
    for (size_t i = 0; i < self->distance_limit; ++i) {
        slot = self->slots + ((hash_index + C1 * i + C2 * i * i) % self->slots_count);
        if (slot->status == vacant) {
            return NULL;
        }
        if (slot->status == occupied && slot->key == key) {
            return slot;
        }
    }

    return NULL;
}

struct hashmap_qp *hashmap_qp_new(uint64_t (*hasher)(uint64_t), void (*value_free)(void *)) {
    struct hashmap_qp *self = malloc(sizeof(struct hashmap_qp));
    self->entries_count = 0;
    self->slots_count = 10;
    self->slots = calloc(self->slots_count, sizeof(struct slot));
    self->distance_limit = log2_64(10);
    self->hasher = hasher;
    self->value_free = value_free;

    return self;
}

bool hashmap_qp_insert(struct hashmap_qp *self, uint64_t key, void *value) {
    if (self == NULL) {
        return false;
    }

    if (1. * self->entries_count / self->slots_count >= MAX_LOAD_FACTOR) {
        resize_map(self);
    }

    uint64_t hash = self->hasher(key);
    while (1) {
        struct slot *slot;
        size_t hash_index = hash % self->slots_count;
        size_t i = 0;
        do {
            slot = self->slots + ((hash_index + C1 * i + C2 * i * i) % self->slots_count);

            if (slot->status != occupied) {
                slot->key = key;
                slot->value = value;
                slot->hash = hash;
                slot->status = occupied;
                self->entries_count++;
                return true;
            }
            if (slot->status == occupied && slot->key == key) {
                self->value_free(slot->value);
                slot->value = value;
                return true;
            }
            ++i;
        } while (i < self->distance_limit);
        resize_map(self);
    }
}

void *hashmap_qp_find(struct hashmap_qp *self, uint64_t key) {
    if (self == NULL) {
        return NULL;
    }

    struct slot *slot = hashmap_qp_find_inner(self, key);
    if (slot == NULL) {
        return NULL;
    } else {
        return slot->value;
    }
}

bool hashmap_qp_delete(struct hashmap_qp *self, uint64_t key) {
    if (self == NULL) {
        return false;
    }

    struct slot *slot = hashmap_qp_find_inner(self, key);
    if (slot == NULL) {
        return false;
    } else {
        self->value_free(slot->value);
        slot->status = released;
        self->entries_count--;
        return true;
    }
}

void hashmap_qp_clear(struct hashmap_qp *self) {
    if (self == NULL) {
        return;
    }

    for (size_t i = 0; i < self->slots_count; ++i) {
        if (self->slots[i].status == occupied) {
            self->value_free(self->slots[i].value);
            self->slots[i].status = vacant;
        }
    }
    self->entries_count = 0;
}

void hashmap_qp_free(struct hashmap_qp *self) {
    if (self == NULL) {
        return;
    }
    for (size_t i = 0; i < self->slots_count; ++i) {
        if (self->slots[i].status == occupied) {
            self->value_free(self->slots[i].value);
        }
    }
    free(self->slots);
    free(self);
}
