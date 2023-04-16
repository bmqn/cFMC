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


static std::optional<std::string> readFile(const std::string &path)
{
    std::ifstream ifs(path);

    if (!ifs.is_open())
    {
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

int main(int argc, char **argv)
{
    std::optional<std::string> exOpt;
    bool showDebug = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        
        if (i < argc - 1)
        {
            if (arg == "--debug")
            {
                showDebug = true;
            }
        }
        else if (i == argc - 1)
        {
            if (auto textOpt = readFile(arg))
            {
                exOpt = textOpt.value();
            }
            else
            {
                std::cerr << "File '" << arg << "' could not be read... ";
                std::cerr << "Please provide a valid file." << std::endl;
                std::cerr << "Usage: cfmc [--debug] <file>" << std::endl;
                return 1;
            }
        }
    }

    if (exOpt)
    {
        const std::string &ex = exOpt.value();

        Parser parser;
        Machine machine;
        machine.execute(parser.parseProgram(ex));
        
        if (showDebug)
        {
            std::cout << std::endl;
            std::cout << machine.getStackDebug();
            std::cout << std::endl;
        }

        return 0;
    }
    else
    {
        std::cerr << "Please provide a file." << std::endl;
        std::cerr << "Usage: cfmc [--debug] <file>" << std::endl;
        return 1;
    }
}