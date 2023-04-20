# cFMC

An interpreter for a C-inspired Functional Machine Calculus (FMC) variant which has first-class locations. This forms part of my dissertation project at the University of Bath which aims to investigate first-class locations in the FMC.

### Programs

Program which pushes (prints) `0` to the output stream. 

```
write = (<@a> . <x> . a[x])
print = ([#out] . write)
main  = ([0] . print)
```

The function `print` is defined in terms of a more general variant called `write` which parameterizes the location to write to and the term to write.

How about a linked-list ? Let's define function `LinkedList` to create a linked-list and function `push_back` to push elements to its tail.

```
LinkedList = (
    <v> . new<@p> . [#null]p . [v]p . [#p]
)

push_back = (
    <v> . <@p> . p<pv> . p<@pp> . [#pp] . (
        null      -> [v] . LinkedList . <@npp> . [#npp]p . [pv]p,
        otherwise -> [#pp]p . [pv]p . [#pp] . [v] . push_back
    )
)
```

Let's also define a function `traverse` for traversing the list in order and running a function `f` on each element.

```
traverse = (
    <f> . <@p> . p<pv> . p<@pp> . [#pp]p . [pv]p . [#pp] . (
        null      -> [pv] . f,
        otherwise -> [pv] . f . [#pp] . [f] . traverse
    )
)
```

Create a linked-list called `p` by calling `LinkedList`, then add elements by calling `push_back`, then print each element by calling `traverse` with the function `print`.

```
print = (
    <x> . [x]out
)

main = (
    [1] . LinkedList . <@p>
    . [#p] . [2] . push_back
    . [#p] . [3] . push_back
    . [#p] . [4] . push_back
    . [#p] . [5] . push_back
    . [#p] . [print] . traverse
)
```

# Running

The program must take a file (containing the program source) or program source directly (but not both). These are given with the options `--file path` or `--source src`.

```
Usage: cfmc [--help] [--debug] [--file path | --source src]
```

For example, running the program in `fibonacci.fmc` would look like.

```
cfmc --file fibonacci.fmc
```

For example, running the program `main = ([in<x> . [x]out] . <echo> . echo)` would look like.

```
cfmc --source 'main = ([in<x> . [x]out] . <echo> . echo)'
```

You can optionally specify `--debug` to display the state of the stack after running the machine.

### macOS & Linux

Execute the included shell script `build.sh` to compile the program. This will generate the binary `cfmc` in the directory `build/`.

### Windows

Execute the included batch script `build.bat` to compile the program. This will generate the binary `cfmc.exe` in the directory `build/`. It will work if executed from the VS Developer Command Prompt. Alternatively, just use WSL !
