use std::{env, path::PathBuf};

extern crate bindgen;
extern crate cc;

fn main() {
    println!("cargo:rerun-if-changed=../implementations/separate_chaining/hashmap_sc.h");
    println!("cargo:rerun-if-changed=../implementations/separate_chaining/hashmap_sc.c");
    println!("cargo:rerun-if-changed=../implementations/linear_probing/hashmap_lp.c");
    println!("cargo:rerun-if-changed=../implementations/linear_probing/hashmap_lp.h");
    println!("cargo:rerun-if-changed=../implementations/quadratic_probing/hashmap_qp.c");
    println!("cargo:rerun-if-changed=../implementations/quadratic_probing/hashmap_qp.h");
    println!("cargo:rerun-if-changed=wrapper.h");
    println!("cargo:rustc-link-lib=static=hashmaps");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindgen::Builder::default()
        .header("wrapper.h")
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");

    let mut build = cc::Build::new();
    build
        .flag("-O3")
        .file("../implementations/separate_chaining/hashmap_sc.c")
        .file("../implementations/linear_probing/hashmap_lp.c")
        .file("../implementations/quadratic_probing/hashmap_qp.c")
        .compile("hashmaps");
}
