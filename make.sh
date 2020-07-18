#!/usr/bin/env sh
cargo build --release || exit 1
mkdir ./bin
cp target/release/libkaktwoos_rs.so ./bin/libkaktwoos_rs.so

g++ -m64 -O3 -o bin/kaktwoos.out main.cc bin/libkaktwoos_rs.so -Wall -Wextra -Iboinc/ -Lboinc/lib/lin -static-libgcc -static-libstdc++ -lboinc_api -lboinc -lboinc_opencl -pthread -Wl,-dynamic-linker,/lib64/ld-linux-x86-64.so.2 || exit 1
