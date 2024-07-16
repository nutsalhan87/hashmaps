#include "../minunit.h"
#include "hashmap_lp.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct hashmap_lp {
    uint64_t entries_count;
    uint64_t slots_count;
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

static uint64_t hasher(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

static uint64_t fake_hasher(uint64_t _) {
    return 1;
}

static void *make_ptr(uint64_t value) {
    uint64_t *p = malloc(sizeof(uint64_t));
    *p = value;
    return p;
}

static void leak(void *_) {}

int tests_run = 0;

static char *test_constructs() {
    struct hashmap_lp *map = hashmap_lp_new(hasher, free);
    mu_assert("error, hashmap constructor returned null", map != NULL);
    mu_assert("error, initial slots count must be equal to 10",
              map->slots_count == 10);
    mu_assert("error, initial entries count must be equal to 0",
              map->entries_count == 0);
    mu_assert("error, array of slots didn't alloc", map->slots != NULL);
    mu_assert("error, distance limit must be equal to 3", map->distance_limit == 3);

    for (size_t i = 0; i < map->slots_count; ++i) {
        mu_assert("error, all slots must be vacant", map->slots[i].status == vacant);
    }

    hashmap_lp_free(map);

    return 0;
}

static char *test_inserts() {
    struct hashmap_lp *map = hashmap_lp_new(fake_hasher, free);
    uint64_t hash = map->hasher(999);
    struct slot *slot = map->slots + (hash % map->slots_count);

    hashmap_lp_insert(map, 999, make_ptr(5));
    mu_assert("error, entries count must be equal to 1", map->entries_count == 1);
    mu_assert("error, map must contain the value", slot != NULL);
    mu_assert("error, saved incorrect hash", slot->hash == hash);
    mu_assert("error, saved incorrect key", slot->key == 999);
    mu_assert("error, saved incorrect value", *(uint64_t *) slot->value == 5);
    mu_assert("error, slot status must be 'occupied'", slot->status == occupied);

    hashmap_lp_insert(map, 999, make_ptr(10));
    mu_assert(
            "error, entries count should be incremented when saving existent key",
            map->entries_count == 1);
    mu_assert("error, value should be changed", *(uint64_t *) slot->value == 10);

    hashmap_lp_insert(map, 777, make_ptr(15));
    mu_assert("error, entries count must be equal to 2", map->entries_count == 2);
    slot++;
    mu_assert("error, new value must be saved at the next slot", *(uint64_t *) slot->value == 15);
    mu_assert("error, new key must be saved at the next slot", slot->key == 777);

    hashmap_lp_free(map);

    return 0;
}

static char *test_finds() {
    struct hashmap_lp *map = hashmap_lp_new(hasher, free);
    hashmap_lp_insert(map, 666, make_ptr(5));
    hashmap_lp_insert(map, 777, make_ptr(10));

    mu_assert("error, map must contain value with key 666",
              *(uint64_t *) hashmap_lp_find(map, 666) == 5);
    mu_assert("error, map must contain value with key 777",
              *(uint64_t *) hashmap_lp_find(map, 777) == 10);
    mu_assert("error, map shouldn't contain value with key 6666",
              hashmap_lp_find(map, 6666) == NULL);

    hashmap_lp_free(map);

    return 0;
}

static char *test_deletes() {
    struct hashmap_lp *map = hashmap_lp_new(hasher, free);
    struct slot *slot = map->slots + (map->hasher(777) % map->slots_count);

    hashmap_lp_insert(map, 555, NULL);
    hashmap_lp_insert(map, 777, NULL);
    mu_assert("error, key 777 must be deleted", hashmap_lp_delete(map, 777));
    mu_assert("error, slot status must be 'released'", slot->status == released);
    mu_assert("error, key 777 already must be deleted", !hashmap_lp_delete(map, 777));
    mu_assert("error, key 888 can't be deleted because the map doesn't contain it",
              !hashmap_lp_delete(map, 888));
    mu_assert("error, entries count must be equal to 1", map->entries_count == 1);

    hashmap_lp_free(map);

    return 0;
}

static char *test_resizes() {
    struct hashmap_lp *map = hashmap_lp_new(fake_hasher, leak);

    for (size_t i = 1; i < 4; ++i) {
        hashmap_lp_insert(map, i, (void *) i);
        mu_assert("error, entries count must be equal to i",
                  map->entries_count == i);
        mu_assert("error, slots count must be equal to 10",
                  map->slots_count == 10);
    }

    hashmap_lp_insert(map, 4, (void *) 4);
    mu_assert("error, entries count must be equal to 4",
              map->entries_count == 4);
    mu_assert("error, slots count must be equal to 20",
              map->slots_count == 20);

    for (size_t i = 1; i <= 4; ++i) {
        mu_assert("error, all previously inserted values must be found", hashmap_lp_find(map, i) == (void *) i);
    }

    hashmap_lp_free(map);

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