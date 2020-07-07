#!/usr/bin/env sh
cargo build --release || exit 1
cp target/release/libkaktwoos_rs.so bin
gcc -O3 -o bin/a.out main.c bin/libkaktwoos_rs.so || exit 1