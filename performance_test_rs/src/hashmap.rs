mod bindings {
    #![allow(non_upper_case_globals)]
    #![allow(non_camel_case_types)]
    #![allow(non_snake_case)]
    #![allow(dead_code)]
    include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
}

use std::hash::{BuildHasher, RandomState};

use libc::c_void;
use once_cell::sync::Lazy;

extern "C" fn hasher(value: u64) -> u64 {
    const HASHER: Lazy<RandomState> = Lazy::new(|| RandomState::new());
    HASHER.hash_one(value)
}

extern "C" fn hasher2(value: u64) -> u64 {
    (hasher(value) << 2) + 1
}

unsafe extern "C" fn free_value<T>(value: *mut c_void) {
    drop(Box::from_raw(value as *mut T));
}

enum HashMapVariant<T> {
    SeparateChaining(*mut bindings::hashmap_sc),
    LinearProbing(*mut bindings::hashmap_lp),
    QuadraticProbing(*mut bindings::hashmap_qp),
    DoubleHashing(*mut bindings::hashmap_dh),
    Std(std::collections::HashMap<u64, T>),
}

pub struct HashMap<T>(HashMapVariant<T>);

impl<T> HashMap<T> {
    pub fn separate_chaining() -> Self {
        Self(HashMapVariant::SeparateChaining(unsafe {
            bindings::hashmap_sc_new(Some(hasher), Some(free_value::<T>))
        }))
    }

    pub fn linear_probing() -> Self {
        Self(HashMapVariant::LinearProbing(unsafe {
            bindings::hashmap_lp_new(Some(hasher), Some(free_value::<T>))
        }))
    }

    pub fn quadratic_probing() -> Self {
        Self(HashMapVariant::QuadraticProbing(unsafe {
            bindings::hashmap_qp_new(Some(hasher), Some(free_value::<T>))
        }))
    }

    pub fn double_hashing() -> Self {
        Self(HashMapVariant::DoubleHashing(unsafe {
            bindings::hashmap_dh_new(Some(hasher), Some(hasher2), Some(free_value::<T>))
        }))
    }

    pub fn std() -> Self {
        Self(HashMapVariant::Std(std::collections::HashMap::new()))
    }

    pub fn label(&self) -> &str {
        match &self.0 {
            HashMapVariant::SeparateChaining(_) => "Separate chaining",
            HashMapVariant::LinearProbing(_) => "Linear probing",
            HashMapVariant::QuadraticProbing(_) => "Quadratic probing",
            HashMapVariant::DoubleHashing(_) => "Double hashing",
            HashMapVariant::Std(_) => "Rust HashMap",
        }
    }

    pub fn insert(&mut self, key: u64, value: T) -> bool {
        match &mut self.0 {
            HashMapVariant::SeparateChaining(ptr) => unsafe {
                bindings::hashmap_sc_insert(
                    *ptr,
                    key,
                    Box::into_raw(Box::new(value)) as *mut c_void,
                )
            },
            HashMapVariant::LinearProbing(ptr) => unsafe {
                bindings::hashmap_lp_insert(
                    *ptr,
                    key,
                    Box::into_raw(Box::new(value)) as *mut c_void,
                )
            },
            HashMapVariant::QuadraticProbing(ptr) => unsafe {
                bindings::hashmap_qp_insert(
                    *ptr,
                    key,
                    Box::into_raw(Box::new(value)) as *mut c_void,
                )
            },
            HashMapVariant::DoubleHashing(ptr) => unsafe {
                bindings::hashmap_dh_insert(
                    *ptr,
                    key,
                    Box::into_raw(Box::new(value)) as *mut c_void,
                )
            },
            HashMapVariant::Std(map) => {
                map.insert(key, value);
                true
            }
        }
    }

    pub fn find(&mut self, key: u64) -> Option<&mut T> {
        match &mut self.0 {
            HashMapVariant::SeparateChaining(ptr) => unsafe {
                (bindings::hashmap_sc_find(*ptr, key) as *mut T).as_mut()
            },
            HashMapVariant::LinearProbing(ptr) => unsafe {
                (bindings::hashmap_lp_find(*ptr, key) as *mut T).as_mut()
            },
            HashMapVariant::QuadraticProbing(ptr) => unsafe {
                (bindings::hashmap_qp_find(*ptr, key) as *mut T).as_mut()
            },
            HashMapVariant::DoubleHashing(ptr) => unsafe {
                (bindings::hashmap_dh_find(*ptr, key) as *mut T).as_mut()
            },
            HashMapVariant::Std(map) => map.get_mut(&key),
        }
    }

    pub fn delete(&mut self, key: u64) -> bool {
        match &mut self.0 {
            HashMapVariant::SeparateChaining(ptr) => unsafe {
                bindings::hashmap_sc_delete(*ptr, key)
            },
            HashMapVariant::LinearProbing(ptr) => unsafe { bindings::hashmap_lp_delete(*ptr, key) },
            HashMapVariant::QuadraticProbing(ptr) => unsafe {
                bindings::hashmap_qp_delete(*ptr, key)
            },
            HashMapVariant::DoubleHashing(ptr) => unsafe { bindings::hashmap_dh_delete(*ptr, key) },
            HashMapVariant::Std(map) => map.remove(&key).is_some(),
        }
    }

    pub fn clear(&mut self) {
        match &mut self.0 {
            HashMapVariant::SeparateChaining(ptr) => unsafe { bindings::hashmap_sc_clear(*ptr) },
            HashMapVariant::LinearProbing(ptr) => unsafe { bindings::hashmap_lp_clear(*ptr) },
            HashMapVariant::QuadraticProbing(ptr) => unsafe { bindings::hashmap_qp_clear(*ptr) },
            HashMapVariant::DoubleHashing(ptr) => unsafe { bindings::hashmap_dh_clear(*ptr) },
            HashMapVariant::Std(map) => map.clear(),
        }
    }
}

impl<T> Drop for HashMap<T> {
    fn drop(&mut self) {
        match &mut self.0 {
            HashMapVariant::SeparateChaining(ptr) => unsafe { bindings::hashmap_sc_free(*ptr) },
            HashMapVariant::LinearProbing(ptr) => unsafe { bindings::hashmap_lp_free(*ptr) },
            HashMapVariant::QuadraticProbing(ptr) => unsafe { bindings::hashmap_qp_free(*ptr) },
            HashMapVariant::DoubleHashing(ptr) => unsafe { bindings::hashmap_dh_free(*ptr) },
            HashMapVariant::Std(_) => (),
        }
    }
}
