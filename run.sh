#!/bin/bash

mkdir -p build

SRC_FILES="
Main.cpp
Lexer.cpp Parser.cpp Machine.cpp
"
echo 'Compiling...'
c++ -std=c++17 -o build/fmc $SRC_FILES

echo 'Running...'
./build/fmc