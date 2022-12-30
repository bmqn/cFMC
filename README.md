# cFMC

A parser and interpreter for the C-like FMC variant discussed in a dissertation project by Max Bryars-Mansell at the University of Bath called called 'An investigation of first-class locations in the FMC'.

# Running

## macOS & Linux

Execute the included shell script `run.sh` to compile and run the program. This will generate the binary `cfmc` in the directory `build/` and execute it.

## Windows

... Don't be silly, use WSL !

# ToDo

- Support brackets
- Runtime warn reporting, i.e. pushing to rnd, reading from out, etc.
- Runtime error reporting, i.e. empty stack, invalid location, etc.