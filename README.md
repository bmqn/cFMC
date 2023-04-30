# cFMC

An interpreter for a C-inspired Functional Machine Calculus (FMC) variant which has first-class locations. This forms part of my dissertation project at the University of Bath which aims to investigate first-class locations in the FMC.

### Programs

#### Basics

Program which pushes (prints) `0` to the output stream. 

```
write = (<@a> . <x> . a[x])
print = ([#out] . write)
main  = ([0] . print)
```

The function `print` is defined in terms of a more general variant called `write` which parameterizes a location to write to and a term to write with.

#### Imperative-style mutable linked-list data structure

Define function `LinkedList` to build a linked-list and define function `push_back` to push elements to its tail.

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

Define function `traverse` for traversing the list in order and running a function `f` on each element.

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

#### Functional-style higher-order function list

Define functions `nil` and `cons` to build a list and define functions `head` and `tail` to access the head and tail of the list, respectively.

```
nil =  (<_> . <x> . [x])
cons = (<h> . <t> . <f> . <x> . [t] . [h] . f)
head = (<l> . [nil] . [<h> . <_> . h] . l)
tail = (<l> . [nil] . [<_> . <t> . t] . l)
```

Define function `map` for lazily mapping the list to a new one which has `f` applied to each element.

```
map = (<f> . <l> . [nil] . [<h> . <t> . [[[t] . [f] . map] . [[h] . f] . cons]] . l)
```

Create a list by building it with `cons`, then map it with `double` to double each element. 

```
double = (<x> . [x] . [x] . +)

main = (
    [[[[nil] . [3] . cons] . [2] . cons] . [1] . cons] . [double] . map . print
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
