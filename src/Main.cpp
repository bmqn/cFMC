#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Program.hpp"
#include "Machine.hpp"
#include "Utils.hpp"

// --- Basics ---

std::string ex00  = "main = ([0] . <x> . [x]out)";
std::string ex01  = "main = (in<x> . [x]out)";
std::string ex03  = "main = ([in<x> . [x]out] . <echo> . echo)";

// -- Function definitions ---

std::string ex10  = R"(
print = (<x> . [x]out)
main  = ([0] . print . [1] . print)
)";

std::string ex11 = R"(
swap = (<x> . <y> . [x] . [y])
main = ([0] . [1] . swap)
)";

std::string ex12 = R"(
swap = (<x> . <y> . [x] . [y])
main = ([[0] . <x> . [x]out] . [[1] . <x> . [x]out] . swap . <f> . f . <f> . f)
)";

// -- Recursion --

std::string ex20 = R"(
true  = (<a> . <b> . a)
false = (<a> . <b> . b)
if    = (<b> . <a> . <p> . [b] . [a] . p)
f     = (<p> . [p] . [in<p> . [p] . f] . [*] . if)
main  = ([true] . f)
)";

// -- First-class locations

std::string ex30 = R"(
write = (<@a> . <x> . [x]a)
print = ([#out] . write)
main  = ([0] . print)
)";

std::string ex31 = R"(
get   = (<@a> . a<x> . [x]a . [x])
write = (<@a> . <x> . [x]a)
print = ([#out] . write)
main  = (new<@a> . [5]a . new<@b> . [2]b . [#a] . get . print . [#b] . get . print)
)";

// -- Data structures

std::string ex40 = R"(
write = (<@a> . <x> . [x]a)
print = ([#out] . write)

Pair  = (<y> . <x> . new<@p> . [y]p . [x]p . [#p])
fst   = (<@p> . p<x> . [x]p . [x])
snd   = (<@p> . p<x> . p<y> . [y]p . [x]p . [y])

main  = ([5] . [2] . Pair . <@p> . [#p] . snd . print . [#p] . fst . print)
)";

std::string ex41 = R"(
write = (<@a> . <x> . [x]a)
print = ([#out] . write)

LinkedList = (
    <v> . new<@p> . [#null]p . [v]p . [#p]
)

push_back = (
    <v> . <@p> . p<pv> . p<@pp> . [#pp] . (
        null      -> [v] . LinkedList . <@npp> . [#npp]p . [pv]p,
        otherwise -> [#pp]p . [pv]p . [#pp] . [v] . push_back
    )
)

traverse = (
    <f> . <@p> . p<pv> . p<@pp> . [#pp]p . [pv]p . [#pp] . (
        null      -> [pv] . f,
        otherwise -> [pv] . f . [#pp] . [f] . traverse
    )
)

rtraverse = (
    <f> . <@p> . p<pv> . p<@pp> . [#pp]p . [pv]p . [#pp] . (
        null      -> [pv] . f,
        otherwise -> [pv] . [#pp] . [f] . rtraverse . f
    )
)

main = (
    [1] . LinkedList . <@p>
    . [#p] . [2] . push_back
    . [#p] . [3] . push_back
    . [#p] . [4] . push_back
    . [#p] . [5] . push_back
    . [#p] . [print] . traverse
    . [#p] . [print] . rtraverse
)
)";

static std::string readFile(const std::string &path)
{
    std::ifstream ifs(path);
    std::stringstream buffer;
    buffer << ifs.rdbuf();

    return buffer.str();
}

int main(int argc, char **argv)
{
    std::optional<std::string> exOpt;
    if (argc == 2)
    {
        std::string path = argv[1];
        exOpt = readFile(path);
    }

    const std::string &ex = exOpt.value_or(ex41);
    // ............... Change example here ^^^^ !

    Parser parser;
    Machine machine;
    machine.execute(parser.parseProgram(ex));

    return 0;
}