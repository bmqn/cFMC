@echo off

set SRC_FILES=Main.cpp Lexer.cpp Term.cpp Parser.cpp Program.cpp Machine.cpp Utils.cpp

echo Compiling...
cl /std:c++20 /DEBUG:FULL /Zi /EHsc /Fo.\build\ /Fd.\build\cfmc.pdb %SRC_FILES% /link /out:build\cfmc.exe

echo Running...
.\build\cfmc.exe