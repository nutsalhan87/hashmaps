#include "hashmap_lp.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOAD_FACTOR 0.7

struct hashmap_lp {
    uint32_t entries_count;
    uint32_t slots_count;
    struct slot *slots;

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

static void resize_if_load_factor_exceeded(struct hashmap_lp *self) {
    if (1. * self->entries_count / self->slots_count < MAX_LOAD_FACTOR) {
        return;
    }

    size_t new_slots_count = 2 * self->slots_count;
    struct slot *new_slots = calloc(new_slots_count, sizeof(struct slot));

    struct slot *old_slots = self->slots;
    for (size_t i = 0; i < self->slots_count; ++i) {
        if (old_slots[i].status != occupied) {
            continue;
        }

        uint64_t new_hash_index = old_slots[i].hash % new_slots_count;
        while (new_slots[new_hash_index].status == occupied) {
            new_hash_index++;
            if (new_hash_index == new_slots_count) {
                new_hash_index = 0;
            }
        }
        memcpy(new_slots + new_hash_index, old_slots + i, sizeof(struct slot));
    }
    free(old_slots);
    self->slots = new_slots;
    self->slots_count *= 2;
}

static struct slot *hashmap_lp_find_inner(struct hashmap_lp *self, uint64_t key) {
    size_t hash_index = self->hasher(key) % self->slots_count;
    struct slot *slot = self->slots + hash_index;
    struct slot *slot_start = slot;
    struct slot *slot_end = self->slots + self->slots_count;

    do {
        if (slot->status == vacant) {
            return NULL;
        }
        if (slot->status == occupied && slot->key == key) {
            return slot;
        }
        slot++;
        if (slot == slot_end) {
            slot = self->slots;
        }
    } while (slot != slot_start);

    return NULL;
}

struct hashmap_lp *hashmap_lp_new(uint64_t (*hasher)(uint64_t), void (*value_free)(void *)) {
    struct hashmap_lp *self = malloc(sizeof(struct hashmap_lp));
    self->entries_count = 0;
    self->slots_count = 10;
    self->slots = calloc(self->slots_count, sizeof(struct slot));
    self->hasher = hasher;
    self->value_free = value_free;

    return self;
}

bool hashmap_lp_insert(struct hashmap_lp *const self, uint64_t key, void *value) {
    if (self == NULL) {
        return false;
    }

    resize_if_load_factor_exceeded(self);

    uint64_t hash = self->hasher(key);
    size_t hash_index = hash % self->slots_count;
    struct slot *slot = self->slots + hash_index;
    struct slot *slot_end = self->slots + self->slots_count;

    while (slot->status == occupied) {
        if (slot->key == key) {
            self->value_free(slot->value);
            slot->value = value;
            return true;
        }

        slot++;
        if (slot == slot_end) {
            slot = self->slots;
        }
    }

    slot->key = key;
    slot->value = value;
    slot->hash = hash;
    slot->status = occupied;

    self->entries_count++;

    return true;
}

void *hashmap_lp_find(struct hashmap_lp *self, uint64_t key) {
    if (self == NULL) {
        return NULL;
    }

    struct slot *slot = hashmap_lp_find_inner(self, key);
    if (slot == NULL) {
        return NULL;
    } else {
        return slot->value;
    }
}

bool hashmap_lp_delete(struct hashmap_lp *self, uint64_t key) {
    if (self == NULL) {
        return false;
    }

    struct slot *slot = hashmap_lp_find_inner(self, key);
    if (slot == NULL) {
        return false;
    } else {
        self->value_free(slot->value);
        slot->status = released;
        self->entries_count--;
        return true;
    }
}

void hashmap_lp_clear(struct hashmap_lp *self) {
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

void hashmap_lp_free(struct hashmap_lp *self) {
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
