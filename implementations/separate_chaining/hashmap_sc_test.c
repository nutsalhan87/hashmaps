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

uint64_t hasher(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

void value_free(void *value) {
    free(value);
}

void *make_ptr(uint64_t value) {
    uint64_t *p = malloc(sizeof(uint64_t));
    *p = value;
    return p;
}

int tests_run = 0;

static char *test_constructs() {
    struct hashmap_sc *map = hashmap_sc_new(hasher, value_free);
    mu_assert("error, hashmap constructor returned null", map != NULL);
    mu_assert("error, initial buckets count must be equal to 10",
              map->buckets_count == 10);
    mu_assert("error, initial entries count must be equal to 0",
              map->entries_count == 0);
    mu_assert("error, array with bucket ref didn't alloc", map->buckets != NULL);

    hashmap_sc_free(map);

    return 0;
}

static char *test_inserts() {
    uint64_t hash = hasher(666);

    struct hashmap_sc *map = hashmap_sc_new(hasher, value_free);
    mu_assert("error, map shouldn't contain any value",
              map->buckets[hash % map->buckets_count].size == 0);

    hashmap_sc_insert(map, 666, make_ptr(5));
    mu_assert("error, entries count must be equal to 1", map->entries_count == 1);

    struct entry *e = map->buckets[hash % map->buckets_count].buffer;
    mu_assert("error, map must contain the value", e != NULL);
    mu_assert("error, saved incorrect hash", e->hash == hash);
    mu_assert("error, saved incorrect key", e->key == 666);
    mu_assert("error, saved incorrect value", *(uint64_t *) e->value == 5);

    hashmap_sc_insert(map, 666, make_ptr(10));
    mu_assert(
            "error, entries count shouldn't be incremented when saving existent key",
            map->entries_count == 1);
    mu_assert("error, value should be changed", *(uint64_t *) e->value == 10);

    for (size_t i = 2; i < 31; ++i) {
        hashmap_sc_insert(map, i, NULL);
        mu_assert("error, entries count must be equal to i",
                  map->entries_count == i);
        mu_assert("error, buckets count must be equal to 10",
                  map->buckets_count == 10);
    }
    hashmap_sc_insert(map, 31, NULL);
    mu_assert("error, entries count must be equal to 31",
              map->entries_count == 31);
    mu_assert("error, buckets count must be equal to 20",
              map->buckets_count == 20);

    hashmap_sc_free(map);

    return 0;
}

static char *test_finds() {
    struct hashmap_sc *map = hashmap_sc_new(hasher, value_free);
    hashmap_sc_insert(map, 666, make_ptr(5));
    hashmap_sc_insert(map, 777, make_ptr(10));

    mu_assert("error, map must contain value with key 777",
              *(uint64_t *) hashmap_sc_find(map, 777) == 10);
    mu_assert("error, map shouldn't contain value with key 6666",
              hashmap_sc_find(map, 6666) == NULL);

    hashmap_sc_free(map);

    return 0;
}

static char *test_deletes() {
    struct hashmap_sc *map = hashmap_sc_new(hasher, value_free);
    hashmap_sc_insert(map, 666, NULL);
    hashmap_sc_insert(map, 777, NULL);
    mu_assert("error, key 777 must be deleted",
              hashmap_sc_delete(map, 777));
    mu_assert("error, key 777 already must be deleted",
              !hashmap_sc_delete(map, 777));
    mu_assert(
            "error, key 888 can't be deleted because the map doesn't contain it",
            !hashmap_sc_delete(map, 888));
    mu_assert("error, entries count must be equal to 1", map->entries_count == 1);

    hashmap_sc_free(map);

    return 0;
}

static char *all_tests() {
    mu_run_test(test_constructs);
    mu_run_test(test_inserts);
    mu_run_test(test_finds);
    mu_run_test(test_deletes);

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