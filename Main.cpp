#include <string>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Program.hpp"
#include "Machine.hpp"

int main()
{
	// --- Basic stream read/write tests ---

	std::string ex0  = "main = (out[x])";
	std::string ex1  = "main = (rnd<x> . out[x])";
	std::string ex2  = "main = (<x> . rnd<y> . out[x] . out[y])";
	std::string ex3  = "main = (rnd<x> . [out[x]] . <f> . f . f)";
	std::string ex4  = "main = ([rnd<x> . out[x]] . <f> . f . f)";
	std::string ex5  = "main = (rnd<x> . [out[x]] . <f> . f . [z] . <x> . f . [x])";
	std::string ex6  = "main = (rnd<x> . [out[x]] . <f> . f . [z] . f . [x])";
	std::string ex7  = "main = ([rnd<x> . out[x]] . <y> . rnd<x> . [out[x] . y] . <f> . f . [z] . <x> . f . out[x])";
	std::string ex8  = "main = ([rnd<x> . out[x]] . <x> . [x] . x . x . rnd[z] . x)";
	std::string ex9  = "main = ([in<x> . out[x]] . <echo> . echo)";

	
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

	std::string ex13 =
		"true  = (<a> . <b> . a)"
		"false = (<a> . <b> . b)"
		"if    = (<b> . <a> . <p> . [b] . [a] . p)"
		"f     = (<p> . [p] . [out[recurse] . in<p> . [p] . f] . [out[exit]] . if)"
		"main  = ([true] . f)";

	// -- First-class locations

	std::string ex14 =
		"main  = (out[dfl] . out[new] . out[in] . out[out])";

	Parser parser;
	Program program = parser.parseProgram(ex14); // <-- Change example here ! 
	program.load([](const FuncDefs_t *funcs) {
		// Don't use funcs outside of this sope.. its life is tied to program !

		Machine machine(funcs);
		machine.execute();
		// machine.printDebug();
	});

	return 0;
}