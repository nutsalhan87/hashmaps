#include <functional>
#include <string>
#include <chrono>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <concepts>

extern "C" {
#include "implementations/separate_chaining/hashmap_sc.h"
#include "implementations/linear_probing/hashmap_lp.h"
#include "implementations/quadratic_probing/hashmap_qp.h"
#include "implementations/double_hashing/hashmap_dh.h"
}

using std::string;
using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::pair;
namespace chrono = std::chrono;

static uint64_t hasher(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

static uint64_t hasher2(uint64_t x) {
    return (hasher(x) << 2) + 1;
}

struct Hasher {
    std::size_t operator()(const uint64_t &value) const noexcept {
        return hasher(value);
    }
};

template<std::destructible T>
void value_free(void *value) {
    delete (T *) value;
}

template<std::destructible T>
class hashmap {
    void *ptr;
    string _label;
    std::function<bool(void *self, uint64_t key, T value)> _insert;
    std::function<T *(void *self, uint64_t key)> _find;
    std::function<bool(void *self, uint64_t key)> _del;
    std::function<void(void *self)> _clear;
    std::function<void(void *self)> _free;

    hashmap() : ptr(nullptr) {}

public:
    static hashmap std() {
        hashmap map;
        map.ptr = new unordered_map<uint64_t, T, Hasher>();
        map._label = "STL";
        map._insert = [](void *self, uint64_t key, T value) {
            ((unordered_map<uint64_t, T, Hasher> *) self)->insert({key, value});
            return true;
        };
        map._find = [](void *self, uint64_t key) {
            auto map = (unordered_map<uint64_t, T, Hasher> *) self;
            auto it = map->find(key);
            return it != map->end() ? &it->second : nullptr;
        };
        map._del = [](void *self, uint64_t key) {
            return ((unordered_map<uint64_t, T, Hasher> *) self)->erase(key) == 1;
        };
        map._clear = [](void *self) { ((unordered_map<uint64_t, T, Hasher> *) self)->clear(); };
        map._free = [](void *self) { delete (unordered_map<uint64_t, T, Hasher> *) self; };

        return map;
    }

    static hashmap sc() {
        hashmap map;
        map.ptr = hashmap_sc_new(hasher, value_free<T>);
        map._label = "Separate chaining";
        map._insert = [](void *self, uint64_t key, T value) {
            auto value_ptr = std::make_unique<T>(std::move(value)).release();
            return hashmap_sc_insert((struct hashmap_sc *) self, key, value_ptr);
        };
        map._find = [](void *self, uint64_t key) { return (T *) hashmap_sc_find((struct hashmap_sc *) self, key); };
        map._del = [](void *self, uint64_t key) { return hashmap_sc_delete((struct hashmap_sc *) self, key); };
        map._clear = [](void *self) { hashmap_sc_clear((struct hashmap_sc *) self); };
        map._free = [](void *self) { hashmap_sc_free((struct hashmap_sc *) self); };

        return map;
    }

    static hashmap lp() {
        hashmap map;
        map.ptr = hashmap_lp_new(hasher, value_free<T>);
        map._label = "Linear probing";
        map._insert = [](void *self, uint64_t key, T value) {
            auto value_ptr = std::make_unique<T>(std::move(value)).release();
            return hashmap_lp_insert((struct hashmap_lp *) self, key, value_ptr);
        };
        map._find = [](void *self, uint64_t key) { return (T *) hashmap_lp_find((struct hashmap_lp *) self, key); };
        map._del = [](void *self, uint64_t key) { return hashmap_lp_delete((struct hashmap_lp *) self, key); };
        map._clear = [](void *self) { hashmap_lp_clear((struct hashmap_lp *) self); };
        map._free = [](void *self) { hashmap_lp_free((struct hashmap_lp *) self); };

        return map;
    }

    static hashmap qp() {
        hashmap map;
        map.ptr = hashmap_qp_new(hasher, value_free<T>);
        map._label = "Quadratic probing";
        map._insert = [](void *self, uint64_t key, T value) {
            auto value_ptr = std::make_unique<T>(std::move(value)).release();
            return hashmap_qp_insert((struct hashmap_qp *) self, key, value_ptr);
        };
        map._find = [](void *self, uint64_t key) { return (T *) hashmap_qp_find((struct hashmap_qp *) self, key); };
        map._del = [](void *self, uint64_t key) { return hashmap_qp_delete((struct hashmap_qp *) self, key); };
        map._clear = [](void *self) { hashmap_qp_clear((struct hashmap_qp *) self); };
        map._free = [](void *self) { hashmap_qp_free((struct hashmap_qp *) self); };

        return map;
    }

    static hashmap dh() {
        hashmap map;
        map.ptr = hashmap_dh_new(hasher, hasher2, value_free<T>);
        map._label = "Double hashing";
        map._insert = [](void *self, uint64_t key, T value) {
            auto value_ptr = std::make_unique<T>(std::move(value)).release();
            return hashmap_dh_insert((struct hashmap_dh *) self, key, value_ptr);
        };
        map._find = [](void *self, uint64_t key) { return (T *) hashmap_dh_find((struct hashmap_dh *) self, key); };
        map._del = [](void *self, uint64_t key) { return hashmap_dh_delete((struct hashmap_dh *) self, key); };
        map._clear = [](void *self) { hashmap_dh_clear((struct hashmap_dh *) self); };
        map._free = [](void *self) { hashmap_dh_free((struct hashmap_dh *) self); };

        return map;
    }

    bool insert(uint64_t key, T value) {
        return this->_insert(this->ptr, key, value);
    }

    T *find(uint64_t key) {
        return this->_find(this->ptr, key);
    }

    bool del(uint64_t key) {
        return this->_del(this->ptr, key);
    }

    void clear() {
        this->_clear(this->ptr);
    }

    string get_label() {
        return this->_label;
    }

    ~hashmap() {
        this->_free(this->ptr);
    }
};

static pair<string, chrono::time_point<chrono::steady_clock>>
inserts_into_new(const std::function<hashmap<uint64_t>()> &map_factory) {
    auto map = map_factory();
    auto start = chrono::steady_clock::now();

    for (uint64_t i = 0; i < 1000000; ++i) {
        map.insert(i, 0);
    }

    return {"1M inserts into new map", start};
}

static pair<string, chrono::time_point<chrono::steady_clock>>
inserts_into_allocated(const std::function<hashmap<uint64_t>()> &map_factory) {
    auto map = map_factory();
    for (uint64_t i = 0; i < 1000000; ++i) {
        map.insert(i, 0);
    }
    map.clear();
    auto start = chrono::steady_clock::now();

    for (uint64_t i = 0; i < 1000000; ++i) {
        map.insert(i, 0);
    }

    return {"1M inserts into already allocated map", start};
}

static pair<string, chrono::time_point<chrono::steady_clock>>
clear(const std::function<hashmap<uint64_t>()> &map_factory) {
    auto map = map_factory();
    for (uint64_t i = 0; i < 1000000; ++i) {
        map.insert(i, 0);
    }
    auto start = chrono::steady_clock::now();

    map.clear();

    return {"Clear map with 1M elements", start};
}

static pair<string, chrono::time_point<chrono::steady_clock>>
deletes(const std::function<hashmap<uint64_t>()> &map_factory) {
    auto map = map_factory();
    for (uint64_t i = 0; i < 100000; ++i) {
        map.insert(i, 0);
    }
    auto start = chrono::steady_clock::now();

    for (uint64_t i = 0; i < 100000; ++i) {
        map.del(i);
    }

    return {"Delete 100k elements one by one", start};
}

static pair<string, chrono::time_point<chrono::steady_clock>>
finds(const std::function<hashmap<uint64_t>()> &map_factory) {
    auto map = map_factory();
    for (uint64_t i = 0; i < 1000000; ++i) {
        map.insert(i, i + 1);
    }
    auto start = chrono::steady_clock::now();

    for (uint64_t i = 0; i < 1000000; ++i) {
        auto res = map.find(i);
        if (*res != i + 1) {
            std::cout << "Result is " << *res << ", but must be " << i + 1 << std::endl;
            exit(2);
        }
    }

    return {"Find 1M elements", start};
}

static pair<string, chrono::time_point<chrono::steady_clock>>
finds_rev(const std::function<hashmap<uint64_t>()> &map_factory) {
    auto map = map_factory();
    for (uint64_t i = 1; i <= 1000000; ++i) {
        map.insert(i, i + 1);
    }
    auto start = chrono::steady_clock::now();

    for (uint64_t i = 1000000; i > 0; --i) {
        auto res = map.find(i);
        if (*res != i + 1) {
            std::cout << "Result is " << *res << ", but must be " << i + 1 << std::endl;
            exit(2);
        }
    }

    return {"Find 1M elements in reverse order", start};
}

static void test(const std::function<hashmap<uint64_t>()> &map_factory) {
    auto tests = {inserts_into_new, inserts_into_allocated, clear, deletes, finds, finds_rev};

    std::cout << "Testing " + map_factory().get_label() << "\n";
    for (auto test: tests) {
        const auto title_and_start = test(map_factory);
        const auto end = chrono::steady_clock::now();
        const chrono::duration<double> elapsed_seconds = end - title_and_start.second;
        std::cout << title_and_start.first << ". Elapsed time: " << std::setw(9) << elapsed_seconds << "\n";
    }
    std::cout << "---------------" << std::endl;
}

int main(int argc, char *argv[]) {
    for (auto map_factory: {
                            hashmap<uint64_t>::std,
                            hashmap<uint64_t>::sc,
                            hashmap<uint64_t>::lp,
                            hashmap<uint64_t>::qp,
                            hashmap<uint64_t>::dh
    }) {
        test(map_factory);
    }

    return 0;
}
