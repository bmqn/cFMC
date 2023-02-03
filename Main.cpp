#include <string>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Program.hpp"
#include "Machine.hpp"

int main()
{
	std::string ex0  = "main = out[x];";
	std::string ex1  = "main = rnd<x> . out[x];";
	std::string ex2  = "main = <x> . rnd<y> . out[x] . out[y];";
	std::string ex3  = "main = rnd<x> . [out[x]] . <f> . f . f;";
	std::string ex4  = "main = [rnd<x> . out[x]] . <f> . f . f;";
	std::string ex5  = "main = rnd<x> . [out[x]] . <f> . f . [hello] . <x> . f . [x];";
	std::string ex6  = "main = rnd<x> . [out[x]] . <f> . f . [hello] . f . [x];";
	std::string ex7  = "main = [rnd<x> . out[x]] . <y> . rnd<x> . [out[x] . y] . <f> . f . [hello] . <x> . f . [x];";
	std::string ex8  = "main = [rnd<x> . out[x]] . <x> . [x] . x . x . rnd[hello] . x;";
	std::string ex9  = "print = <x> . out[x]; main = [zero] . print . [one] . print;";
	std::string ex10 = "swap = <x> . <y> . [x] . [y]; main = [zero] . [one] . swap;";

	Parser parser;

	Program program = parser.parse(ex10);
	program.load([](const FuncDefs_t *funcs) {
		// Don't use funcs outside of this sope.. its life is tied to program !

		Machine machine(funcs);
		machine.execute();
		machine.printDebug();
	});

	return 0;
}