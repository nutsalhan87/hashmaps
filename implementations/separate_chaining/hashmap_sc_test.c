#include "../minunit.h"
#include "hashmap_sc.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

static uint64_t hasher(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

static void *make_ptr(uint64_t value) {
    uint64_t *p = malloc(sizeof(uint64_t));
    *p = value;
    return p;
}

static void leak(void *_) {}

int tests_run = 0;

static char *test_constructs() {
    struct hashmap_sc *map = hashmap_sc_new(hasher, free);
    mu_assert("error, hashmap constructor returned null", map != NULL);
    mu_assert("error, initial buckets count must be equal to 10",
              map->buckets_count == 10);
    mu_assert("error, initial entries count must be equal to 0",
              map->entries_count == 0);
    mu_assert("error, array with bucket ref didn't alloc", map->buckets != NULL);

    for (size_t i = 0; i < map->buckets_count; ++i) {
        mu_assert("error, all buckets initially must be unallocated", map->buckets[i].buffer == NULL);
        mu_assert("error, all buckets initially must have size 0", map->buckets[i].size == 0);
        mu_assert("error, all buckets initially must have capacity 0", map->buckets[i].capacity == 0);
    }

    hashmap_sc_free(map);

    return 0;
}

static char *test_inserts() {
    struct hashmap_sc *map = hashmap_sc_new(hasher, free);
    uint64_t hash = map->hasher(666);

    hashmap_sc_insert(map, 666, make_ptr(5));
    mu_assert("error, entries count must be equal to 1", map->entries_count == 1);

    struct entry *entry = map->buckets[hash % map->buckets_count].buffer;
    mu_assert("error, map must contain the value", entry != NULL);
    mu_assert("error, saved incorrect hash", entry->hash == hash);
    mu_assert("error, saved incorrect key", entry->key == 666);
    mu_assert("error, saved incorrect value", *(uint64_t *) entry->value == 5);

    hashmap_sc_insert(map, 666, make_ptr(10));
    mu_assert(
            "error, entries count shouldn't be incremented when saving existent key",
            map->entries_count == 1);
    mu_assert("error, value should be changed", *(uint64_t *) entry->value == 10);

    hashmap_sc_insert(map, 777, make_ptr(15));
    entry = map->buckets[hash % map->buckets_count].buffer + 1;
    mu_assert("error, entries count must be equal to 2", map->entries_count == 2);
    mu_assert("error, new value must be saved at the next slot", *(uint64_t *) entry->value == 15);
    mu_assert("error, new key must be saved at the next slot", entry->key == 777);

    hashmap_sc_free(map);

    return 0;
}

static char *test_finds() {
    struct hashmap_sc *map = hashmap_sc_new(hasher, free);
    hashmap_sc_insert(map, 666, make_ptr(5));
    hashmap_sc_insert(map, 777, make_ptr(10));

    mu_assert("error, map must contain value with key 666",
              *(uint64_t *) hashmap_sc_find(map, 666) == 5);
    mu_assert("error, map must contain value with key 777",
              *(uint64_t *) hashmap_sc_find(map, 777) == 10);
    mu_assert("error, map shouldn't contain value with key 6666",
              hashmap_sc_find(map, 6666) == NULL);

    hashmap_sc_free(map);

    return 0;
}

static char *test_deletes() {
    struct hashmap_sc *map = hashmap_sc_new(hasher, free);
    struct bucket *bucket = map->buckets + (map->hasher(666) % map->buckets_count);

    hashmap_sc_insert(map, 777, NULL);
    hashmap_sc_insert(map, 666, NULL);
    mu_assert("error, key 777 must be deleted",
              hashmap_sc_delete(map, 777));
    mu_assert("error, key 777 already must be deleted",
              !hashmap_sc_delete(map, 777));
    mu_assert(
            "error, key 888 can't be deleted because the map doesn't contain it",
            !hashmap_sc_delete(map, 888));
    mu_assert("error, entries count must be equal to 1", map->entries_count == 1);
    mu_assert("error, entry with deleted key must be replaced with last entry in the bucket", bucket->buffer->key == 666);
    mu_assert("error, entries count in the bucket must be equal to 1", bucket->size == 1);

    hashmap_sc_free(map);

    return 0;
}

static char *test_resizes() {
    struct hashmap_sc *map = hashmap_sc_new(hasher, leak);

    for (size_t i = 1; i < 31; ++i) {
        hashmap_sc_insert(map, i, (void *) i);
        mu_assert("error, entries count must be equal to i",
                  map->entries_count == i);
        mu_assert("error, buckets count must be equal to 10",
                  map->buckets_count == 10);
    }
    hashmap_sc_insert(map, 31, (void *) 31);
    mu_assert("error, entries count after resizing must be equal to 31",
              map->entries_count == 31);
    mu_assert("error, buckets count after resizing must be equal to 20",
              map->buckets_count == 20);

    for (size_t i = 1; i <= 31; ++i) {
        mu_assert("error, all previously inserted values must be found", hashmap_sc_find(map, i) == (void *) i);
    }

    hashmap_sc_free(map);

    return 0;
}

static char *all_tests() {
    mu_run_test(test_constructs);
    mu_run_test(test_inserts);
    mu_run_test(test_finds);
    mu_run_test(test_deletes);
    mu_run_test(test_resizes);

    return NULL;
}

int main() {
    char *result = all_tests();
    if (result != NULL) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != NULL;
}