@echo off

set SRC_FILES=src\Main.cpp src\Lexer.cpp src\Term.cpp src\Parser.cpp src\Program.cpp src\Machine.cpp src\Utils.cpp

echo Compiling...
cl /std:c++20 /DEBUG:FULL /Zi /EHsc /Fo.\build\ /Fd.\build\cfmc.pdb %SRC_FILES% /link /out:build\cfmc.exe

echo Done...!