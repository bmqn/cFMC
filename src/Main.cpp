#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Program.hpp"
#include "Machine.hpp"

static std::string readFile(const std::string &path)
{
	std::ifstream ifs(path);
	std::stringstream buffer;
	buffer << ifs.rdbuf();

	return buffer.str();
}

int main()
{
	// --- Basics ---

	std::string ex0  = "main = ([0] . out[x])";
	std::string ex1  = "main = (in<x> . out[x])";
	std::string ex3  = "main = ([in<x> . out[x]] . <echo> . echo)";
	
	// -- Function definitions ---
	
	std::string ex10  =
		"print = (<x> . out[x])"
		"main  = ([zero] . print . [one] . print)";
	
	std::string ex11 =
		"swap = (<x> . <y> . [x] . [y])"
		"main = ([zero] . [one] . swap)";
	
	std::string ex12 =
		"swap = (<x> . <y> . [x] . [y])"
		"main = ([[zero] . <x> . out[x]] . [[one] . <x> . out[x]] . swap . <f> . f . <f> . f)";

	// -- Recursion --

	std::string ex20 =
		"true  = (<a> . <b> . a)"
		"false = (<a> . <b> . b)"
		"if    = (<b> . <a> . <p> . [b] . [a] . p)"
		"f     = (<p> . [p] . [out[recurse] . in<p> . [p] . f] . [out[exit]] . if)"
		"main  = ([true] . f)";

	// -- First-class locations

	std::string ex30 =
		"write = (<a> . <x> . a[x])"
		"print = ([out] . write)"
		"main  = ([xyz] . print)";

	std::string ex31 =
		"main  = ([new] . <a> . a<b> . out[b] . b[hello] . b<x> . out[x] . out[a])";

	std::string ex32 =
		"write = (<a> . <x> . a[x])"
		"true  = (<a> . <b> . a)"
		"false = (<a> . <b> . b)"
		"if    = (<b> . <a> . <p> . [b] . [a] . p)"
		"main = ([xyz] . in<p> . [p] . [[out]] . [new<a> . [a]] . if . write)";

	// -- Data structures

	std::string ex40 = readFile("linked_list.fmc");

	{
		Parser parser;
		Program program = parser.parseProgram(ex40); // <-- Change example here ! 
		program.load([](const FuncDefs_t *funcs) {
			Machine machine(funcs);
			machine.execute();
			// machine.printDebug(); // <-- Enable debug print to see stack & binds
		});
	}

	return 0;
}