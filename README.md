# cFMC

A parser and interpreter for a C-inspired Functional Machine Calculus (FMC) variant which has first-class locations. This forms part of my dissertation project at the University of Bath which aims to investigate first-class locations in the FMC.


### Programs

Program which pushes (prints) `0` to the output stream. 

```
write = (<a> . <x> . a[x])
print = ([out] . write)
main  = ([0] . print)
```

The function `print` is defined in terms of a more general variant called `write` which parameterizes the location to write to and the term to write.


# Running

### macOS & Linux

Execute the included shell script `run.sh` to compile and run the program. This will generate the binary `cfmc` in the directory `build/` and execute it.

### Windows

... Don't be silly, use WSL !
