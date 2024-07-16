#include "hashmap_dh.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOAD_FACTOR 70

struct hashmap_dh {
    uint64_t entries_count;
    uint64_t slots_count;
    struct slot *slots;
    uint64_t distance_limit;

    uint64_t (*hasher1)(uint64_t);

    uint64_t (*hasher2)(uint64_t);

    void (*value_free)(void *);
};

enum slot_status {
    vacant = 0,
    occupied,
    released
};

struct slot {
    uint64_t hash1;
    uint64_t hash2;
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

static void resize_map(struct hashmap_dh *const self) {
    size_t new_slots_count = 2 * self->slots_count;
    struct slot *new_slots = calloc(new_slots_count, sizeof(struct slot));

    struct slot *old_slots = self->slots;
    for (size_t i = 0; i < self->slots_count; ++i) {
        if (old_slots[i].status != occupied) {
            continue;
        }

        uint64_t hash1 = old_slots[i].hash1;
        uint64_t hash2 = old_slots[i].hash2;
        uint64_t new_hash_index = hash1 % new_slots_count;
        for (size_t j = 1; new_slots[new_hash_index].status == occupied; ++j) {
            new_hash_index = (hash1 + hash2 * j) % new_slots_count;
        }
        memcpy(new_slots + new_hash_index, old_slots + i, sizeof(struct slot));
    }
    free(old_slots);
    self->slots = new_slots;
    self->slots_count *= 2;
    self->distance_limit = log2_64(new_slots_count);
}

static struct slot *find_inner(struct hashmap_dh *const self, uint64_t key) {
    uint64_t hash1 = self->hasher1(key);
    uint64_t hash2 = self->hasher2(key);
    struct slot *slot = self->slots + hash1 % self->slots_count;
    for (size_t i = 1; i <= self->distance_limit; ++i) {
        if (slot->status == vacant) {
            return NULL;
        }
        if (slot->status == occupied && slot->key == key) {
            return slot;
        }
        slot = self->slots + (hash1 + hash2 * i) % self->slots_count;
    }

    return NULL;
}

struct hashmap_dh *
hashmap_dh_new(uint64_t (*hasher1)(uint64_t), uint64_t (*hasher2)(uint64_t), void (*value_free)(void *)) {
    struct hashmap_dh *self = malloc(sizeof(struct hashmap_dh));
    self->entries_count = 0;
    self->slots_count = 10;
    self->slots = calloc(self->slots_count, sizeof(struct slot));
    self->distance_limit = log2_64(10);
    self->hasher1 = hasher1;
    self->hasher2 = hasher2;
    self->value_free = value_free;

    return self;
}

bool hashmap_dh_insert(struct hashmap_dh *const self, uint64_t key, void *value) {
    if (self == NULL) {
        return false;
    }

    if (100 * self->entries_count / self->slots_count >= MAX_LOAD_FACTOR) {
        resize_map(self);
    }

    uint64_t hash1 = self->hasher1(key);
    uint64_t hash2 = self->hasher2(key);
    while (1) {
        struct slot *slot = self->slots + hash1 % self->slots_count;
        for (size_t i = 1; i <= self->distance_limit; ++i) {
            if (slot->status != occupied) {
                slot->key = key;
                slot->value = value;
                slot->hash1 = hash1;
                slot->hash2 = hash2;
                slot->status = occupied;
                self->entries_count++;
                return true;
            }
            if (slot->status == occupied && slot->key == key) {
                self->value_free(slot->value);
                slot->value = value;
                return true;
            }
            slot = self->slots + (hash1 + hash2 * i) % self->slots_count;
        }
        resize_map(self);
    }
}

void *hashmap_dh_find(struct hashmap_dh *const self, uint64_t key) {
    if (self == NULL) {
        return NULL;
    }

    struct slot *slot = find_inner(self, key);
    if (slot == NULL) {
        return NULL;
    } else {
        return slot->value;
    }
}

bool hashmap_dh_delete(struct hashmap_dh *const self, uint64_t key) {
    if (self == NULL) {
        return false;
    }

    struct slot *slot = find_inner(self, key);
    if (slot == NULL) {
        return false;
    } else {
        self->value_free(slot->value);
        slot->status = released;
        self->entries_count--;
        return true;
    }
}

void hashmap_dh_clear(struct hashmap_dh *const self) {
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

void hashmap_dh_free(struct hashmap_dh *const self) {
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

