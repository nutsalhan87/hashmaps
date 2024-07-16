#include "hashmap_sc.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOAD_FACTOR 3

struct hashmap_sc {
    uint32_t entries_count;
    uint32_t buckets_count;
    struct bucket *buckets;

    uint64_t (*hasher)(uint64_t);

    void (*value_free)(void *);
};

struct bucket {
    uint32_t size;
    uint32_t capacity;
    struct entry *buffer;
};

struct entry {
    uint64_t hash;
    uint64_t key;
    void *value;
};

static void resize_if_load_factor_exceeded(struct hashmap_sc *const self) {
    if (1. * self->entries_count / self->buckets_count < MAX_LOAD_FACTOR) {
        return;
    }

    uint32_t new_buckets_count = 2 * self->buckets_count;
    struct bucket *new_buckets =
            calloc(new_buckets_count, sizeof(struct bucket));
    memcpy(new_buckets, self->buckets, self->buckets_count * sizeof(struct bucket));
    for (size_t i = 0; i < self->buckets_count; ++i) {
        struct bucket *iter = new_buckets + i;
        for (size_t j = 0; j < iter->size; ++j) {
            size_t new_hash_index = iter->buffer[j].hash % new_buckets_count;
            if (new_hash_index == i) {
                continue;
            }

            struct entry current = iter->buffer[j];
            if (iter->size - 1 != j) {
                iter->buffer[j] = iter->buffer[iter->size - 1];
            }
            iter->size--;
            j--;

            struct bucket *bucket = new_buckets + new_hash_index;
            if (bucket->buffer == NULL) {
                bucket->size = 0;
                bucket->capacity = 1;
                bucket->buffer = malloc(sizeof(struct entry));
            } else if (bucket->size == bucket->capacity) {
                bucket->capacity *= 2;
                bucket->buffer = reallocarray(bucket->buffer, bucket->capacity, sizeof(struct entry));
            }

            bucket->buffer[bucket->size] = current;
            bucket->size++;
        }
    }

    self->buckets_count = new_buckets_count;
    free(self->buckets);
    self->buckets = new_buckets;
}

static struct entry *find_inner(struct hashmap_sc *const self, uint64_t key) {
    size_t hash_index = self->hasher(key) % self->buckets_count;
    struct bucket b = self->buckets[hash_index];
    for (size_t i = 0; i < b.size; ++i) {
        if (b.buffer[i].key == key) {
            return b.buffer + i;
        }
    }
    return NULL;
}

struct hashmap_sc *hashmap_sc_new(uint64_t (*hasher)(uint64_t), void (*value_free)(void *)) {
    struct hashmap_sc *self = malloc(sizeof(struct hashmap_sc));
    self->entries_count = 0;
    self->buckets_count = 10;
    self->hasher = hasher;
    self->buckets = calloc(self->buckets_count, sizeof(struct bucket));
    self->value_free = value_free;

    return self;
}

bool hashmap_sc_insert(struct hashmap_sc *const self, uint64_t key, void *value) {
    if (self == NULL) {
        return false;
    }

    struct entry *c = find_inner(self, key);
    if (c != NULL) {
        self->value_free(c->value);
        c->value = value;
        return true;
    }

    resize_if_load_factor_exceeded(self);

    uint64_t hash = self->hasher(key);
    size_t hash_index = hash % self->buckets_count;
    struct entry new_entry = (struct entry) {
            .key = key,
            .value = value,
            .hash = hash
    };

    struct bucket *bucket = self->buckets + hash_index;
    if (bucket->buffer == NULL) {
        bucket->size = 0;
        bucket->capacity = 1;
        bucket->buffer = malloc(sizeof(struct entry));
    } else if (bucket->size == bucket->capacity) {
        bucket->capacity *= 2;
        bucket->buffer = reallocarray(bucket->buffer, bucket->capacity, sizeof(struct entry));
    }

    bucket->buffer[bucket->size] = new_entry;
    bucket->size++;
    self->entries_count++;

    return true;
}

void *hashmap_sc_find(struct hashmap_sc *const self, uint64_t key) {
    if (self == NULL) {
        return NULL;
    }

    struct entry *e = find_inner(self, key);
    if (e == NULL) {
        return NULL;
    } else {
        return e->value;
    }
}

bool hashmap_sc_delete(struct hashmap_sc *const self, uint64_t key) {
    if (self == NULL) {
        return false;
    }

    size_t hash_index = self->hasher(key) % self->buckets_count;
    struct bucket *bucket = self->buckets + hash_index;
    for (size_t i = 0; i < bucket->size; ++i) {
        if (bucket->buffer[i].key != key) {
            continue;
        }
        self->value_free(bucket->buffer[i].value);
        if (bucket->size - 1 != i) {
            bucket->buffer[i] = bucket->buffer[bucket->size - 1];
        }
        bucket->size--;
        self->entries_count--;
        return true;
    }

    return false;
}

void hashmap_sc_clear(struct hashmap_sc *const self) {
    if (self == NULL) {
        return;
    }

    for (size_t i = 0; i < self->buckets_count; ++i) {
        struct bucket *bucket = self->buckets + i;
        for (size_t j = 0; j < bucket->size; ++j) {
            self->value_free(bucket->buffer[j].value);
        }
        bucket->size = 0;
    }
    self->entries_count = 0;
}

void hashmap_sc_free(struct hashmap_sc *const self) {
    if (self == NULL) {
        return;
    }

    for (size_t i = 0; i < self->buckets_count; ++i) {
        struct bucket *bucket = self->buckets + i;
        for (size_t j = 0; j < bucket->size; ++j) {
            self->value_free(bucket->buffer[j].value);
        }
        free(bucket->buffer);
    }
    free(self->buckets);
    free(self);
}
