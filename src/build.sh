#!/bin/bash

mkdir -p ../build
pushd ../build
pwd

rm -f compile_commands.json

# bear -- clang ../src/sdl_handmade.cpp -o handmade `sdl2-config --cflags --libs`
bear -- clang ../src/sdl_handmade.cpp -o handmade $(sdl2-config --cflags --libs)
popd
