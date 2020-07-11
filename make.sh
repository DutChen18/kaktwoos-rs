#!/usr/bin/env sh
cargo build --release || exit 1
cp target/release/libkaktwoos_rs.so bin
g++ -O3 -o bin/a.out main.cc bin/libkaktwoos_rs.so -Wall -Wextra || exit 1