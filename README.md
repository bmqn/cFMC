# cFMC

A parser and interpreter for a C-inspired Functional Machine Calculus (FMC) variant which has first-class locations. This forms part of my dissertation project at the University of Bath which aims to investigate first-class locations in the FMC.


### Programs

Program which pushes (prints) `0` to the output stream. 

```
write = (<^a> . <x> . a[x])
print = ([#out] . write)
main  = ([0] . print)
```

The function `print` is defined in terms of a more general variant called `write` which parameterizes the location to write to and the term to write.

How about a linked-list ? Let's define function `LinkedList` to create a linked-list and function `push_back` to push elements to its tail.

```
LinkedList = (
    <v> . new<^p> . [#null]p . [v]p . [#p]
)

push_back = (
    <v> . <^p> . p<pv> . p<^pp> . [#pp] . (
        null      -> [v] . LinkedList . <^np> . [#np]pp . [v]pp,
        otherwise -> [v]p . [#pp] . [#pp] . [v] . push_back
    )
)
```

Let's also define a function `traverse` for traversing every element in order and calling a function `f` on each.

```
traverse = (
    <f> . <^p> . p<pv> . p<^pp> . [#pp]p . [pv]p . [#pp] . (
        null      -> [pv] . f,
        otherwise -> [pv] . f . [#pp] . [f] . traverse
    )
)
```

Create a linked-list with head pointer `hp` by calling `LinkedList`, and add elements by calling `push_back`, and print each element by calling `traverse` with the function `<x>.[x]out`.

```
main = (
    [1] . LinkedList . <^p>
    . [#p] . [2] . push_back
    . [#p] . [3] . push_back
    . [#p] . [4] . push_back
    . [#p] . [5] . push_back
    . [#p] . [<x>.[x]out] . traverse
)
```

# Running

### macOS & Linux

Execute the included shell script `run.sh` to compile and run the program. This will generate the binary `cfmc` in the directory `build/` and execute it.

### Windows

... Don't be silly, use WSL ! There is `run.bat` which will work if executed from the VS Developer Command Prompt.
