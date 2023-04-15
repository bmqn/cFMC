#!/bin/bash

mkdir -p build

SRC_FILES="src/Main.cpp src/Lexer.cpp src/Term.cpp src/Parser.cpp src/Program.cpp src/Machine.cpp src/Utils.cpp"

echo 'Compiling...'
c++ -std=c++20 -g -o build/cfmc $SRC_FILES

echo 'Done...!'