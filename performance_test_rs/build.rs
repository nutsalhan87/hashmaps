use std::{env, path::PathBuf};

extern crate bindgen;
extern crate cc;

fn main() {
    let mut build = cc::Build::new();
    build.flag("-O3");

    for source in [
        "separate_chaining/hashmap_sc",
        "linear_probing/hashmap_lp",
        "quadratic_probing/hashmap_qp",
        "double_hashing/hashmap_dh",
    ] {
        println!("cargo:rerun-if-changed=../implementations/{}.h", source);
        println!("cargo:rerun-if-changed=../implementations/{}.c", source);
        build.file(format!("../implementations/{}.c", source));
    }
    println!("cargo:rerun-if-changed=wrapper.h");
    println!("cargo:rustc-link-lib=static=hashmaps");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindgen::Builder::default()
        .header("wrapper.h")
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");

    build.compile("hashmaps");
}
