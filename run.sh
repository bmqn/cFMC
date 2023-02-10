#!/bin/bash

mkdir -p build

SRC_FILES="
Main.cpp
Lexer.cpp Term.cpp Parser.cpp Program.cpp Machine.cpp Utils.cpp
"
echo 'Compiling...'
c++ -g -std=c++20 -o build/cfmc $SRC_FILES

echo 'Running...'
stdbuf -oL ./build/cfmc