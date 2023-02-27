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

How about a linked-list ? Let's define function `create` to create a linked-list and function `push_back` to push elements to its tail.

```
create = (
    <v> . new<hp> . hp[v] . hp[0] . [hp]
)

push_back = (
    <v> . <hp> . hp<hnp> . hp<hv> . [hnp] . (
        0         -> [v] . create . <np> . hp[hv] . hp[np],
        otherwise -> hp[hv] . hp[hnp] . [hnp] . [v] . push_back
    )
)
```

Let's also define a function `traverse` for traversing every element in order and calling a function `f` on each.

```
traverse = (
    <f> . <hp> . hp<hnp> . hp<hv> . hp[hv] . hp[hnp] . [hnp] . (
        0         -> [hv] . f,
        otherwise -> [hv] . f . [hnp] . [f] . traverse
    )
)
```

Create a linked-list with head pointer `hp` by calling `create`, and add elements by calling `push_back`, and print each element by calling `traverse` with the function `<x>.out[x]`.

```
main = (
    [1] . create . <hp>
    . [hp] . [2] . push_back
    . [hp] . [3] . push_back
    . [hp] . [<x> . out[x]] . traverse
)
```

# Running

### macOS & Linux

Execute the included shell script `run.sh` to compile and run the program. This will generate the binary `cfmc` in the directory `build/` and execute it.

### Windows

... Don't be silly, use WSL ! There is `run.bat` which will work if executed from the VS Developer Command Prompt.
