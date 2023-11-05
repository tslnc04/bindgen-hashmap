# bindgen-hashmap

A hashmap implemented in C with bindings and a safe API in Rust.

## Why?

Mostly a learning exercise for better understanding hashmaps while also learning
about `bindgen`. It's not intended for use, although one could.

## Usage

Tests may be run with `cargo nextest run`. Otherwise, the following may be put
in `Cargo.toml` to use the library:

```toml
[dependencies]
bindgen-hashmap = { git = "https://github.com/tslnc04/bindgen-hashmap.git" }
```
