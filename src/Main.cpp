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

struct Args
{
	std::string Source;
	bool Debug = false;
};

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

static Args parseArgs(int argc, char **argv)
{
	Args args;
	bool isSrcSpecified = false;

	auto fail = [](std::string msg) {
		std::cerr << msg << std::endl;
		std::cerr << "Usage: cfmc [--help] [--debug] [--file path | --source src]" << std::endl;
		std::exit(1);
	};

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		
		if (arg == "--help")
		{
			fail("Parses & interprets programs written in the cFMC !");
		}
		else if (arg == "--debug")
		{
			args.Debug = true;
		}
		if (arg == "--file" && !isSrcSpecified)
		{
			if (i + 1 < argc)
			{
				std::string path = argv[i + 1];
				if (auto sourceOpt = readFile(path))
				{
					args.Source = sourceOpt.value();
					isSrcSpecified = true;
				}
				else
				{
					fail("File '" + path + "' could not be read... "
						"Please provide a valid file."
					);
				}
			}
			else
			{
				fail("Expected path after '--file'.");
			}
		}
		else if (arg == "--source" && !isSrcSpecified)
		{
			if (i + 1 < argc)
			{
				std::string source = argv[i + 1];
				args.Source = source;
				isSrcSpecified = true;
			}
			else
			{
				fail("Expected term after '--source'.");
			}
		}
	}

	if (!isSrcSpecified)
	{
		fail("No file or source is specified.");
	}

	return args;
}

int main(int argc, char **argv)
{
	auto args = parseArgs(argc, argv);

	Parser parser;
	Machine machine;
	machine.execute(parser.parseProgram(args.Source));
	
	if (args.Debug)
	{
		std::cout << std::endl;
		std::cout << machine.getStackDebug();
		std::cout << std::endl;
	}
}