use std::time::Instant;

use hashmap::HashMap;

mod hashmap;

fn inserts_into_new(map_factory: &fn() -> HashMap<u64>) -> (&'static str, Instant) {
    let mut map = map_factory();
    let start = Instant::now();

    for i in 0..1000000 {
        map.insert(i, 0);
    }

    return ("1M inserts into new map", start);
}

fn inserts_into_alocated(map_factory: &fn() -> HashMap<u64>) -> (&'static str, Instant) {
    let mut map = map_factory();
    for i in 0..1000000 {
        map.insert(i, 0);
    }
    map.clear();
    let start = Instant::now();

    for i in 0..1000000 {
        map.insert(i, 0);
    }

    return ("1M inserts into already allocated map", start);
}

fn clear(map_factory: &fn() -> HashMap<u64>) -> (&'static str, Instant) {
    let mut map = map_factory();
    for i in 0..1000000 {
        map.insert(i, 0);
    }
    let start = Instant::now();

    map.clear();

    return ("Clear map with 1M elements", start);
}

fn deletes(map_factory: &fn() -> HashMap<u64>) -> (&'static str, Instant) {
    let mut map = map_factory();
    for i in 0..100000 {
        map.insert(i, 0);
    }
    let start = Instant::now();

    for i in 0..100000 {
        map.delete(i);
    }

    return ("Delete 100k elements one by one", start);
}

fn finds(map_factory: &fn() -> HashMap<u64>) -> (&'static str, Instant) {
    let mut map = map_factory();
    for i in 0..1000000 {
        map.insert(i, i + 1);
    }
    let mut wrong_counter: usize = 0;
    let start = Instant::now();

    for i in 0..1000000 {
        map.find(i).inspect(|v| {
            if **v != i + 1 {
                wrong_counter += 1;
            }
        });
    }
    if wrong_counter != 0 {
        eprint!("Found {} wrong values. ", wrong_counter);
    }

    return ("Find 1M elements", start);
}

fn finds_rev(map_factory: &fn() -> HashMap<u64>) -> (&'static str, Instant) {
    let mut map = map_factory();
    for i in 0..1000000 {
        map.insert(i, i + 1);
    }
    let mut wrong_counter: usize = 0;
    let start = Instant::now();

    for i in (0..1000000).rev() {
        map.find(i).inspect(|v| {
            if **v != i + 1 {
                wrong_counter += 1;
            }
        });
    }
    if wrong_counter != 0 {
        eprint!("Found {} wrong values. ", wrong_counter);
    }

    return ("Find 1M elements in reverse order", start);
}

fn main() {
    let tests = &[
        inserts_into_new,
        inserts_into_alocated,
        clear,
        deletes,
        finds,
        finds_rev,
    ];
    let map_factories: &[fn() -> HashMap<u64>] = &[
        HashMap::std,
        HashMap::separate_chaining,
        HashMap::linear_probing,
        HashMap::quadratic_probing,
        HashMap::double_hashing,
    ];

    for map_factory in map_factories {
        println!("Testing {}", map_factory().label());
        for test in tests {
            let (test_title, start) = test(map_factory);
            let elapsed = Instant::now().duration_since(start);
            println!(
                "{}. Elapsed time: {:.6}s.",
                test_title,
                elapsed.as_secs_f32()
            )
        }
        println!("---------------");
    }
}
